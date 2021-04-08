
#ifndef  MDS_TASK_INNER_H
#define  MDS_TASK_INNER_H


/* �������ȼ� */
#define HIGHESTPRIO         0                   /* ����������ȼ� */
#define LOWESTPRIO          (PRIORITYNUM - 1)   /* ����������ȼ� */


/* �����־ */
#define DELAYQUEFLAG        0x00000001  /* ������delay���еı�־, 0: ����, 1: �� */


/*********************************** �������� *************************************/
#ifdef MDS_INCLUDETASKHOOK
extern VFHCRT gvfTaskCreateHook;
extern VFHSWT gvfTaskSwitchHook;
extern VFHDLT gvfTaskDeleteHook;
#endif


/*********************************** �������� *************************************/
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

