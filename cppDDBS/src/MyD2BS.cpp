//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory.h>
#include <windows.h>
#include <string.h>
#include <vector>
#include <iterator>
#include <math.h>
#include "GlobalVer.h"
#include "GlobalDef.h"

//using namespace std;

/* char bit-mask rule Reading */
int ReadChbitRules() {
	printf(">> ReadChbitRules... \n");

	FILE* fp;
	struct CHAR_RULE_SET* ptrRuleset;
	char validfilter; /* validfilter means an '@'*/
	ptrRuleset = &gRuleset;
	ptrRuleset->numRule = 0;

//	char filename[10] = "acl.txt";	/* filter file name */
	fp = fopen(".\\chbit_4k.txt", "r");
	if (fp == NULL) {
		printf("Couldnt open filter set file \n");
		return FAILURE;
	}

	while (!feof(fp)) {
		fscanf(fp, "%c", &validfilter);
		if (validfilter != '@')
			continue; /* each rule should begin with an '@' */

		for(unsigned int i=0;i<32;i++){
			fscanf(fp,"%c",&ptrRuleset->ruleList[ptrRuleset->numRule].chbit[i]);
		}


//				!= fscanf(fp,
//						"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"),
//						&ptrRuleset->ruleList[ptrRuleset->numRule]) {
//			printf("\n>> [err] ill-format IP slash rule-file\n");
//			exit(-1);
//		}

		ptrRuleset->numRule++;

	}

	fclose(fp);
	printf(">> Rules Account: %u \n", ptrRuleset->numRule);

	return SUCCESS;
}

/* Initialization */
int Initial(struct DBS_NODE* ptrNode) {
//	char tmp_mask[32]="0000000000000000";
//	memcpy(gMask,tmp_mask,32*sizeof(char));

//	gMask[32] = "0000000000000000";

	ptrNode = &gRootnode;
	ptrNode->nbit = 0;

	ptrNode->child[0] = NULL;
	ptrNode->child[1] = NULL;

	return SUCCESS;
}

/* binary rule partition for only one bit */
int RuleBRP(struct CHAR_RULE* ptrChRule, unsigned int pos,
		struct DBS_NODE * leftNode, struct DBS_NODE * rightNode) {

	if (ptrChRule->chbit[pos] != '0') {
		rightNode->ruleset->numRule++;
		memcpy(&(rightNode->ruleset->ruleList[rightNode->ruleset->numRule]),
				ptrChRule, sizeof(struct CHAR_RULE));
	}

	if (ptrChRule->chbit[pos] != '1') {
		leftNode->ruleset->numRule++;
		memcpy(&(leftNode->ruleset->ruleList[leftNode->ruleset->numRule]),
				ptrChRule, sizeof(struct CHAR_RULE));
	}

	return SUCCESS;
}

/* DBS tree node partition */
int DBScurrNode(struct DBS_NODE * currNode, unsigned int pos) {

	struct DBS_NODE leftNode;
	struct DBS_NODE rightNode;

	for (unsigned int i = 0; i < currNode->ruleset->numRule; i++) {
		RuleBRP(&(currNode->ruleset->ruleList[i]), pos, &leftNode, &rightNode);
	}

	if (leftNode.ruleset->numRule == 0 || rightNode.ruleset->numRule == 0) {
		return FAILURE;
	}

	currNode->child[0] = &leftNode;
	currNode->child[1] = &rightNode;

	return SUCCESS;
}

/*calculating the number of bit for mask*/
unsigned int CalcMaskNbit(char* mask) {
	unsigned int nbit = 0;
	for (unsigned int i; i < 32; i++) {
		if (mask[i] == '1') {
			nbit++;
		}
	}

	return nbit;
}

/*calculating the jfactor when fast-growth and intelligent swap*/
unsigned int CalcJFactor(struct CHAR_RULE_SET* chRuleSet, char* mask) {

	/*definition*/
	unsigned int jfactor, nbit, nblock, num_insert,exp;
	unsigned int insert_bindx[MAX_BLOCK_NUM]; //insert index of block

	/*initialization*/
	nbit = CalcMaskNbit(mask);
	nblock = 0x1 << nbit;

	unsigned int tmp_block_incrulenum[nblock];
	for (unsigned int x = 0; x < nblock; x++) {
		tmp_block_incrulenum[x] = 0;
	}



	/*tracing included rules*/
	for (unsigned int rindx = 0; rindx < chRuleSet->numRule; rindx++) {

		/*init*/
		exp=0;
		num_insert=1;
		for(unsigned int y = 0; y < MAX_BLOCK_NUM; y++){
			insert_bindx[y]=0;
		}

		/*tracing pos of mask*/
		for (unsigned int maskbit = 0; maskbit < 32; maskbit++) {
			if (mask[maskbit] == '0') {
				continue;
			}

			if (chRuleSet->ruleList[rindx].chbit[maskbit] == '0') {
				continue;
			}
			if (chRuleSet->ruleList[rindx].chbit[maskbit] == '1') {
				for (unsigned int z = 0; z<num_insert; z++) {
					insert_bindx[z] += 0x1 << exp;
				}

			}
			if (chRuleSet->ruleList[rindx].chbit[maskbit] == '*') {

				/*generated a copy when '*' */
				for (unsigned int i = 0; i < num_insert; i++) {
					insert_bindx[i+num_insert] = insert_bindx[i];
				}

				for (unsigned int j = 0; j < num_insert; j++) {
					insert_bindx[j] += 0x1 << exp;
				}

				num_insert = num_insert << 1;
			}

			exp++;
		}

		/*set rulenum per block*/
		for (unsigned int z = 0; z < num_insert; z++) {
			tmp_block_incrulenum[insert_bindx[z]]++;
		}

	}



	/*set jfactor as the maximum rulenum */
	jfactor = 0x0;
	for (unsigned int t = 0; t < nblock; t++) {
		if (tmp_block_incrulenum[t] > jfactor) {
			jfactor = tmp_block_incrulenum[t];
		}
	}

	return jfactor;
}

/*fast-growth stage of ddbs*/
int FastGrowth(struct CHAR_RULE_SET* chRuleSet) {

	/*definition*/
	unsigned int pfactor,pupper;
	unsigned int cntDux[chRuleSet->numRule];

	/*initialization*/
	pupper=DUPLICATION * chRuleSet->numRule;
	for (unsigned int x = 0; x < chRuleSet->numRule; x++) {
		cntDux[x] = 0x1;
	}

	pfactor = 0;
	/*calculating pfactor*/
	for (unsigned int i = 0; i < 32; i++) {

		if (pfactor < pupper) {
			/*num dubel when **/
			for (unsigned int rindx = 0; rindx < chRuleSet->numRule; rindx++) {
				if (chRuleSet->ruleList[rindx].chbit[i] == '*') {
					cntDux[rindx] = cntDux[rindx] << 1;
				}
			}

			pfactor = 0;
			for (unsigned int j = 0; j < chRuleSet->numRule; j++) {
				pfactor = pfactor + cntDux[j];
			}

			gMask[i] = '1';
		}else{
//			printf("pfactor > PUPPER !!\n");
			gMask[i] = '0';
		}

	}

	printf(">> fast-growth mask is %s\n", gMask);
	return SUCCESS;
}

/*Intelligent swap stage of ddbs*/
int IntellSwap(struct CHAR_RULE_SET* chRuleSet, char* mask) {

	/*definition*/
	unsigned int cnt_0 = 0;
	unsigned int cnt_1 = 0;
	unsigned int indx_0[32];
	unsigned int indx_1[32];
	char tmp_mask[32];
	unsigned int tmp_jfactor;

	/*generating list '0' and '1'*/
	for (unsigned int mindx = 0; mindx < 32; mindx++) {
		if (mask[mindx] == '0') {
			indx_0[cnt_0] = mindx;
			cnt_0++;
		}
		if (mask[mindx] == '1') {
			indx_1[cnt_1] = mindx;
			cnt_1++;
		}
	}


	/*get origin jfacotr*/
	memcpy(tmp_mask, mask, 32 * sizeof(char));
	unsigned int jfactor;
	jfactor = CalcJFactor(chRuleSet, mask);

	/*swapping '0' and '1' to find minimum jfactor*/
	for (unsigned int i = 0; i < cnt_0; i++) {
		for (unsigned int j = 0; j < cnt_1; j++) {
//			memcpy(tmp_mask, mask, 32 * sizeof(char));
			tmp_mask[indx_0[i]] = '1';
			tmp_mask[indx_1[j]] = '0';

			tmp_jfactor = CalcJFactor(chRuleSet, tmp_mask);
			if (tmp_jfactor < jfactor) {
				jfactor = tmp_jfactor;
				memcpy(mask, tmp_mask, 32 * sizeof(char));
			}
			tmp_mask[indx_0[i]] = '0';
			tmp_mask[indx_1[j]] = '1';
		}
	}

	return SUCCESS;
}

/*generating table*/
int GenerateTB(struct CHAR_RULE_SET* ptrChRuleSet, char* mask,
		struct DBS_TABLE *ptrTable) {
	/*	ptrTable=&gTable;
	 * 	mask=&gMask;
	 * 	ptrChRuleSet=&gRuleset;
	 */

	/*definition*/
	unsigned int nbit, nblock, bindx, exp;
	unsigned int insert_block_indx[MAX_BLOCK_NUM];
	unsigned int num_insert = 0x1;

	/*initialization*/
	nbit = CalcMaskNbit(mask);
	nblock = 0x1 << nbit;
	ptrTable->numBlock = nblock;

	ptrTable->pblockList = (struct DBS_BLOCK*) malloc(
			ptrTable->numBlock * sizeof(struct DBS_BLOCK));
	if (ptrTable->pblockList == NULL) {
		printf("malloc fail...");
		exit(0);
	}

	for (unsigned int x = 0; x < ptrTable->numBlock; x++) {
		ptrTable->pblockList[x].numRule = 0;
	}

	/*tracing included rules*/
	for (unsigned int rindx = 0; rindx < ptrChRuleSet->numRule; rindx++) {

		/*init*/
		num_insert=1;
		exp=0;
		for (unsigned int y = 0; y < MAX_BLOCK_NUM; y++) {
			insert_block_indx[y] = 0;
		}

		/*tracing pos of mask*/
		for (unsigned int maskbit = 0; maskbit < MAX_NBIT; maskbit++) {
			if (mask[maskbit] == '0') {
				continue;
			}
			if (ptrChRuleSet->ruleList[rindx].chbit[maskbit] == '0') {
				continue;
			}
			if (ptrChRuleSet->ruleList[rindx].chbit[maskbit] == '1') {
				for (unsigned int i = 0; i < num_insert; i++) {
					insert_block_indx[i] += 0x1 << exp;
				}
			}
			if (ptrChRuleSet->ruleList[rindx].chbit[maskbit] == '*') {

				/*generated a copy when **/
				for (unsigned int i = 0; i < num_insert; i++) {
					insert_block_indx[num_insert + i] = insert_block_indx[i];
				}

				for (unsigned int j = 0; j < num_insert; j++) {
					insert_block_indx[j] += 0x1 << exp;
				}
				num_insert = num_insert << 1;
			}

			exp++;
		}

		/*inserting rindx*/
		for (unsigned int z = 0; z < num_insert; z++) {
			bindx = insert_block_indx[z];

			memcpy(&(ptrTable->pblockList[bindx].ruleList[ptrTable->pblockList[bindx].numRule]),
					&(ptrChRuleSet->ruleList[rindx]),
					32*sizeof(char));

			ptrTable->pblockList[bindx].numRule++;

		}
	}

	return SUCCESS;
}

int Statistic() {


	unsigned int sum_rule_num=0;
	unsigned int max_rule_num=0;
	float dux,avg_rule_num;
	for(unsigned int i =0; i<gTable.numBlock;i++){
		sum_rule_num+=gTable.pblockList[i].numRule;
		if(gTable.pblockList[i].numRule>max_rule_num){
			max_rule_num=gTable.pblockList[i].numRule;
		}
	}
	dux= sum_rule_num / gRuleset.numRule;
	avg_rule_num=sum_rule_num / gTable.numBlock;


	printf(">> gMask is %s\n",gMask);
	printf(">> nblock is %u\n",gTable.numBlock);
	printf(">> sum_rule_num of block is %u\n", sum_rule_num);
	printf(">> max_rule_num of block is %u\n", max_rule_num);
	printf(">> avg_rule_num of block is %f\n", avg_rule_num);
	printf(">> duplication is %f\n", dux);



	return SUCCESS;
}

int main(int argc, char* argv[]) {

	int iret;
	iret = ReadChbitRules();
	iret = FastGrowth(&gRuleset);
	iret = IntellSwap(&gRuleset, gMask);
	iret = GenerateTB(&gRuleset, gMask, &gTable);
	iret = Statistic();

	return SUCCESS;
}

