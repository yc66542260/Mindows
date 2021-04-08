
#ifndef  MDS_TASK_INNER_H
#define  MDS_TASK_INNER_H


/* 任务优先级 */
#define HIGHESTPRIO         0                   /* 任务最高优先级 */
#define LOWESTPRIO          (PRIORITYNUM - 1)   /* 任务最低优先级 */


/* 任务标志 */
#define DELAYQUEFLAG        0x00000001  /* 任务在delay表中的标志, 0: 不在, 1: 在 */


/*********************************** 变量声明 *************************************/
#ifdef MDS_INCLUDETASKHOOK
extern VFHCRT gvfTaskCreateHook;
extern VFHSWT gvfTaskSwitchHook;
extern VFHDLT gvfTaskDeleteHook;
#endif


/*********************************** 函数声明 *************************************/
extern M_TCB* MDS_TaskTcbInit(U8* pucTaskName, VFUNC vfFuncPointer,
          U8* pucTaskStack, U32 uiStackSize, U8 ucTaskPrio, M_TASKOPT* pstrTaskOpt);
extern U32 MDS_TaskPend(M_SEM* pstrSem, U32 uiDelayTick);
extern void MDS_SwitchToTask(void);
extern void MDS_TaskOccurSwi(U32 uiSwiNo);
extern U32 MDS_GetChipMode(void);
extern void MDS_IntEnable(void);
extern void MDS_IntDisable(void);

#ifdef MDS_INCLUDETASKHOOK
extern void MDS_TaskHookInit(void);
#endif


#endif

