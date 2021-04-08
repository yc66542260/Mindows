
#ifndef  MDS_CORE_C_H
#define  MDS_CORE_C_H


/*********************************** º¯ÊýÉùÃ÷ *************************************/
extern U32 MDS_TickTimerInit(VFUNC vfFuncPointer);
extern U32 MDS_SystemTickGet(void);
extern M_TCB* MDS_CurrentTcbGet(void);
extern U32 MDS_IntLock(void);
extern U32 MDS_IntUnlock(void);
extern void MDS_ChainInit(M_CHAIN* pstrChain);
extern void MDS_ChainNodeAdd(M_CHAIN* pstrChain, M_CHAIN* pstrNode);
extern M_CHAIN* MDS_ChainNodeDelete(M_CHAIN* pstrChain);
extern void MDS_ChainCurNodeInsert(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                                   M_CHAIN* pstrNewNode);
extern M_CHAIN* MDS_ChainCurNodeDelete(M_CHAIN* pstrChain, M_CHAIN* pstrNode);
extern M_CHAIN* MDS_ChainEmpInq(M_CHAIN* pstrChain);
extern M_CHAIN* MDS_ChainNextNodeEmpInq(M_CHAIN* pstrChain, M_CHAIN* pstrNode);


#endif

