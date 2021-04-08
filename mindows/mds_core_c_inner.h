
#ifndef  MDS_CORE_C_INNER_H
#define  MDS_CORE_C_INNER_H


/*********************************** 变量声明 *************************************/
extern U8 gaucRootTaskStack[ROOTTASKSTACK];
extern U8 gaucIdleTaskStack[IDLETASKSTACK];
extern M_TASKSCHEDTAB gstrReadyTab;
extern M_CHAIN gstrDelayTab;
extern U32 guiTick;
extern STACKREG* gpstrCurTaskSpAddr;
extern STACKREG* gpstrNextTaskSpAddr;
extern M_TCB* gpstrCurTcb;
extern M_TCB* gpstrRootTaskTcb;
extern M_TCB* gpstrIdleTaskTcb;
extern U32 guiIntLockCounter;
extern const U8 caucTaskPrioUnmapTab[256];


/*********************************** 函数声明 *************************************/
extern void MDS_SystemVarInit(void);
extern void MDS_TaskTick(void);
extern void MDS_TaskSwitch(M_TCB* pstrTcb);
extern void MDS_TaskStart(M_TCB* pstrTcb);
extern void MDS_TaskSchedTabInit(M_TASKSCHEDTAB* pstrSchedTab);
extern void MDS_TaskAddToSchedTab(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                                  M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio);
extern void MDS_TaskAddToDelayTab(M_CHAIN* pstrNode);
extern M_CHAIN* MDS_TaskDelFromSchedTab(M_CHAIN* pstrChain,
                                        M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio);
extern void MDS_TaskAddToSemTab(M_TCB* pstrTcb, M_SEM* pstrSem);
extern M_CHAIN* MDS_TaskDelFromSemTab(M_TCB* pstrTcb);
extern M_TCB* MDS_SemGetActiveTask(M_SEM* pstrSem);
extern void MDS_TaskSched(void);
extern void MDS_TaskReadySched(void);
extern M_TCB* MDS_TaskReadyTabSched(void);
extern void MDS_TaskDelayTabSched(void);
extern void MDS_TaskPrioFlagSet(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio);
extern void MDS_TaskPrioFlagClr(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio);
extern U8 MDS_TaskHighestPrioGet(M_PRIOFLAG* pstrPrioFlag);


#endif

