/*
 * global_def.h
 *
 *  Created on: 2015Äê9ÔÂ24ÈÕ
 *      Author: 10177270
 */

#ifndef GLOBAL_DEF_H_
#define GLOBAL_DEF_H_


#include <vector>
#include <iterator>

/*************************** Macro Definition *********************/

#define DEBUG_MALLOC
//#define	DEBUG
#define	LOOKUP
//#define	ShowResults
//#define FULLHS
#define HSINC 128

#define FILTER_FILE ".\\filter\\table_acl1_10k"

/* for 5-tuple classification */
#define DIM			5

/* for function return value */
#define SUCCESS		0
#define FAILURE		-1
#define TRUE		1
#define FALSE		0


#define MAX_BLOCK_NUM	0xffff
#define MAX_RULE_NUM	65536
#define WORDLENGTH	32	/* for 32-bit system */
#define BITMAPSIZE	256 /* MAXFILTERS/WORDLENGTH */
#define DUPLICATION 4
#define MAX_NBIT 32 // 16k




/*define function*/
#define random(x) (rand()%x)



/*-----------------------------------------------------------------------------
 *  structure
 *-----------------------------------------------------------------------------*/
struct CHAR_RULE {
	unsigned char chbit[32];
};

struct CHAR_RULE_SET {
	unsigned int numRule;
	struct CHAR_RULE ruleList[MAX_RULE_NUM];
};


struct DBS_NODE {
	unsigned int nbit;
	struct CHAR_RULE_SET* ruleset;
	struct DBS_NODE* child[2]; /* pointer to child-node, 2 for binary split */

};



struct DBS_BLOCK {
	unsigned int numRule;
	struct CHAR_RULE ruleList[MAX_RULE_NUM];
};


struct DBS_TABLE {
	unsigned int numBlock;
	struct DBS_BLOCK * pblockList;
};





/*

struct IP {
//	unsigned int pri;
	unsigned int ip[DIM];
};

struct IPSET {
	unsigned int num;  number of rules in the rule set
	struct IP* ipList;  rules in the set
};

*/




struct STASTIC{
	unsigned int	nblock;
	unsigned int	leafRuleMalloc;
};





/*************************** Profile *********************/
extern unsigned long long dff;
extern unsigned long c1;
extern unsigned long c2;
extern LARGE_INTEGER  large_interger;

#define PROFILE_START 		\
						do{\
							QueryPerformanceFrequency(&large_interger);\
							dff = large_interger.QuadPart;\
							QueryPerformanceCounter(&large_interger);\
							c1 = large_interger.QuadPart;\
						}while(0)


#define PROFILE_END 		\
						do{\
							QueryPerformanceCounter(&large_interger);\
							c2 = large_interger.QuadPart;\
						}while(0)

/*-----------------------------------------------------------------------------
 *  global
 *-----------------------------------------------------------------------------*/
extern struct CHAR_RULE_SET gRuleset;
extern struct DBS_NODE gRootnode;
extern struct STASTIC gStatistic;
extern char gMask[32];















#endif /* GLOBAL_DEF_H_ */
