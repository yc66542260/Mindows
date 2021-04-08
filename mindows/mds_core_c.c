
#include "mindows_inner.h"


M_TASKSCHEDTAB gstrReadyTab;            /* ��������� */
M_CHAIN gstrDelayTab;                   /* �����ӳٱ� */

U32 guiTick;                            /* ����ϵͳ��tick���� */

STACKREG* gpstrCurTaskSpAddr;           /* ��ǰ���е�����ļĴ�����ָ��, ���ڴ�C����
                                           ����� */
STACKREG* gpstrNextTaskSpAddr;          /* ��Ҫ���е�����ļĴ�����ָ��, ���ڴ�C����
                                           ����� */

M_TCB* gpstrCurTcb;                     /* ��ǰ���е������TCBָ�� */
M_TCB* gpstrRootTaskTcb;                /* root����TCBָ�� */
M_TCB* gpstrIdleTaskTcb;                /* idle����TCBָ�� */

U32 guiIntLockCounter;                  /* ���жϼ���ֵ */

const U8 caucTaskPrioUnmapTab[256] =    /* ���ȼ�������ұ� */
{
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};


/***********************************************************************************
��������: ������, C����Ӵ˺�����ʼ����. �ú�������ϵͳ���񲢴ӷ�����״̬�л���root
          ��������.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
S32 main(void)
{
    /* ��ʼ��ϵͳ���� */
    MDS_SystemVarInit();

    /* ���������� */
    gpstrRootTaskTcb = MDS_TaskCreate((U8*)"Root", (VFUNC)MDS_RootTask,
                   gaucRootTaskStack, ROOTTASKSTACK, HIGHESTPRIO, (M_TASKOPT*)NULL);

    /* ������������ */
    gpstrIdleTaskTcb = MDS_TaskCreate((U8*)"Idle", (VFUNC)MDS_IdleTask,
                    gaucIdleTaskStack, IDLETASKSTACK, LOWESTPRIO, (M_TASKOPT*)NULL);

    /* ��ʼ�������, �Ӹ�����ʼִ�� */
    MDS_TaskStart(gpstrRootTaskTcb);

    return 0;
}

/***********************************************************************************
��������: ��ʼ��tick��ʱ��.
��ڲ���: vfFuncPointer: ��ʼ��tickʱ�ӵĺ���ָ��.
�� �� ֵ: RTN_SUCD: ��ʼ��tickʱ�ӳɹ�.
          RTN_FAIL: ��ʼ��tickʱ��ʧ��.
***********************************************************************************/
U32 MDS_TickTimerInit(VFUNC vfFuncPointer)
{
    /* ��ڲ������ */
    if(NULL == vfFuncPointer)
    {
        return RTN_FAIL;
    }

    /* ��ʼ��tickϵͳʱ�� */
    vfFuncPointer();

    return RTN_SUCD;
}

/***********************************************************************************
��������: ��ʼ��ϵͳ����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_SystemVarInit(void)
{
    /* ����ǰ��������, root�����idle�����TCB��ʼ��ΪNULL */
    gpstrCurTcb = (M_TCB*)NULL;
    gpstrRootTaskTcb = (M_TCB*)NULL;
    gpstrIdleTaskTcb = (M_TCB*)NULL;

    /* ��ʼ��tick��0��ʼ */
    guiTick = 0;

    /* ��ʼ�����жϼ���Ϊ0 */
    guiIntLockCounter = 0;

#ifdef MDS_INCLUDETASKHOOK

    /* ��ʼ�����Ӻ��� */
    MDS_TaskHookInit();

#endif

    /* ��ʼ������ready�� */
    MDS_TaskSchedTabInit(&gstrReadyTab);

    /* ��ʼ������delay�� */
    MDS_ChainInit(&gstrDelayTab);
}

/***********************************************************************************
��������: tick�жϵ���������.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskTick(void)
{
    /* ÿ��tick�ж�tick������1 */
    guiTick++;

    /* �������� */
    MDS_TaskSched();
}

/***********************************************************************************
��������: Ϊ�����������л���׼��, ���������л������л�ຯ����ʹ�õı���.
��ڲ���: pstrTcb: �������е������TCBָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSwitch(M_TCB* pstrTcb)
{
    /* ���浱ǰ�����TCB���ݼĴ������ַ, �ڻ�����Կ���ͨ����������ҵ�����ı��ݼ�
       �������ַ */
    gpstrCurTaskSpAddr = &gpstrCurTcb->strStackReg;

    /* ���漴�����������TCB���ݼĴ������ַ, �ڻ�����Կ���ͨ����������ҵ�����ı�
       �ݼĴ������ַ */
    gpstrNextTaskSpAddr = &pstrTcb->strStackReg;

    /* ���漴�����������TCBָ�� */
    gpstrCurTcb = pstrTcb;
}

/***********************************************************************************
��������: ��ʼ�����������.
��ڲ���: pstrTcb: �������е������TCBָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskStart(M_TCB* pstrTcb)
{
    /* ���漴�����������ջָ��ĵ�ַ, �ڻ�����Կ���ͨ�����������ȡ�����л�������
       ��Ķ�ջ��ַ */
    gpstrNextTaskSpAddr = &pstrTcb->strStackReg;

    /* ���漴�����������TCBָ�� */
    gpstrCurTcb = pstrTcb;

    MDS_SwitchToTask();
}

/***********************************************************************************
��������: ��ʼ��������ȱ�.
��ڲ���: pstrSchedTab: ���ȱ�ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSchedTabInit(M_TASKSCHEDTAB* pstrSchedTab)
{
    U32 i;

    /* ��ʼ��ÿ�����ȼ�������ready���� */
    for(i = 0; i < PRIORITYNUM; i++)
    {
        MDS_ChainInit(&pstrSchedTab->astrChain[i]);
    }

    /* ��ʼ��ready�����ȼ���־ */
#if PRIORITYNUM >= PRIORITY128

    for(i = 0; i < PRIOFLAGGRP1; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp1[i] = 0;
    }

    for(i = 0; i < PRIOFLAGGRP2; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp2[i] = 0;
    }

    pstrSchedTab->strFlag.ucPrioFlagGrp3 = 0;

#elif PRIORITYNUM >= PRIORITY16

    for(i = 0; i< PRIOFLAGGRP1; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp1[i] = 0;
    }

    pstrSchedTab->strFlag.ucPrioFlagGrp2 = 0;

#else

    pstrSchedTab->strFlag.ucPrioFlagGrp1 = 0;

#endif
}

/***********************************************************************************
��������: ��������ӵ�������ȱ�.
��ڲ���: pstrChain: ���ȱ�ָ��, ��������ӵ��õ��ȱ�.
          pstrNode: Ҫ��ӵ�����ڵ�ָ��.
          pstrPrioFlag: �õ��ȱ��Ӧ�����ȼ���־��ָ��.
          ucTaskPrio: Ҫ��ӵ���������ȼ�.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskAddToSchedTab(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                           M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
    /* ��������ڵ���ӵ����ȱ��� */
    MDS_ChainNodeAdd(pstrChain, pstrNode);

    /* ���ø������Ӧ���ȱ�����ȼ���־�� */
    MDS_TaskPrioFlagSet(pstrPrioFlag, ucTaskPrio);
}

/***********************************************************************************
��������: ��������ӵ�delay��, ���������ӳ�ʱ����ٵ�����ӵ�delay����.
��ڲ���: pstrNode: Ҫ��ӵ�����ڵ�ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskAddToDelayTab(M_CHAIN* pstrNode)
{
    M_CHAIN* pstrTempNode;
    M_CHAIN* pstrNextNode;
    M_TCBQUE* pstrDelayQue;
    U32 uiStillTick;
    U32 uiTempStillTick;

    /* ��ȡdelay���еĵ�һ������ڵ�ָ�� */
    pstrTempNode = MDS_ChainEmpInq(&gstrDelayTab);

    /* delay��ǿ� */
    if(NULL != pstrTempNode)
    {
        /* ��ȡҪ����delay���������ӳ�ʱ�� */
        pstrDelayQue = (M_TCBQUE*)pstrNode;
        uiStillTick = pstrDelayQue->pstrTcb->uiStillTick;

        /* Ѱ��Ҫ����delay���������delay���е�λ�� */
        while(1)
        {
            /* ��ȡdelay����������ӳ�ʱ�� */
            pstrDelayQue = (M_TCBQUE*)pstrTempNode;
            uiTempStillTick = pstrDelayQue->pstrTcb->uiStillTick;

            /* �����¸��������ж�Ҫ����delay�������Ӧ�ò����λ�� */
            if(uiStillTick < uiTempStillTick)
            {
                if(uiStillTick > guiTick)
                {
                    MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode, pstrNode);

                    return;
                }
                else if(uiStillTick < guiTick)
                {
                    if(guiTick > uiTempStillTick)
                    {
                        MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode,
                                               pstrNode);

                        return;
                    }
                }
            }
            else if(uiStillTick > uiTempStillTick)
            {
                if(uiStillTick > guiTick)
                {
                    if(guiTick > uiTempStillTick)
                    {
                        MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode,
                                               pstrNode);

                        return;
                    }
                }
            }

            /* delay����ȡ���������ӳ�ʱ��С��Ҫ����delay��������ӳ�ʱ��, ȡ���¸�
               ����ڵ�����ж� */
            pstrNextNode = MDS_ChainNextNodeEmpInq(&gstrDelayTab, pstrTempNode);

            /* ������delay�������һ���ڵ�, ��Ҫ����delay���������뵽delay��β */
            if(NULL == pstrNextNode)
            {
                MDS_ChainNodeAdd(&gstrDelayTab, pstrNode);

                return;
            }
            else /* ����delay���¸�����ڵ�����ж� */
            {
                pstrTempNode = pstrNextNode;
            }
        }
    }
    else /* delay��Ϊ����ֱ�ӽ�Ҫ����delay���������뵽delay��β */
    {
        MDS_ChainNodeAdd(&gstrDelayTab, pstrNode);

        return;
    }
}

/***********************************************************************************
��������: ��������ȱ�ɾ������.
��ڲ���: pstrChain: ���ȱ�ָ��, �Ӹ�����ɾ������.
          pstrPrioFlag: ���ȱ��Ӧ�����ȼ���־��ָ��.
          ucTaskPrio: Ҫɾ����������ȼ�.
�� �� ֵ: ��ɾ��������Ľڵ�ָ��.
***********************************************************************************/
M_CHAIN* MDS_TaskDelFromSchedTab(M_CHAIN* pstrChain, M_PRIOFLAG* pstrPrioFlag,
                                 U8 ucTaskPrio)
{
    M_CHAIN* pstrDelNode;

    /* ��������ڵ�ӵ��ȱ���ɾ�� */
    pstrDelNode = MDS_ChainNodeDelete(pstrChain);

    /* ������ȱ��и����ȼ�Ϊ����������ȼ���־ */
    if(NULL == MDS_ChainEmpInq(pstrChain))
    {
        MDS_TaskPrioFlagClr(pstrPrioFlag, ucTaskPrio);
    }

    /* ���ر�ɾ������Ľڵ�ָ�� */
    return pstrDelNode;
}

/***********************************************************************************
��������: ��������ӵ�sem���ȱ�.
��ڲ���: pstrTcb: �����TCB.
          pstrSem: �������ź���ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskAddToSemTab(M_TCB* pstrTcb, M_SEM* pstrSem)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* ����ź����ǲ������ȼ������㷨 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* ��ȡ�������ز��� */
        ucTaskPrio = pstrTcb->ucTaskPrio;
        pstrChain = &pstrSem->strSemTab.astrChain[ucTaskPrio];
        pstrNode = &pstrTcb->strTcbQue.strQueHead;
        pstrPrioFlag = &pstrSem->strSemTab.strFlag;

        /* ��ӵ�sem���ȱ� */
        MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);
    }
    else /* ����ź��������Ƚ��ȳ������㷨 */
    {
        /* ��ȡ�������ز���, ʹ��0���ȼ�������Ϊ�Ƚ��ֳ����� */
        pstrChain = &pstrSem->strSemTab.astrChain[LOWESTPRIO];
        pstrNode = &pstrTcb->strTcbQue.strQueHead;

        /* ��ӵ�sem���ȱ� */
        MDS_ChainNodeAdd(pstrChain, pstrNode);
    }
}

/***********************************************************************************
��������: �������sem���ȱ�ɾ��.
��ڲ���: pstrTcb: �����TCB.
�� �� ֵ: ��ɾ������Ľڵ�ָ��.
***********************************************************************************/
M_CHAIN* MDS_TaskDelFromSemTab(M_TCB* pstrTcb)
{
    M_SEM* pstrSem;
    M_CHAIN* pstrChain;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* ��ȡ����������ź��� */
    pstrSem = pstrTcb->pstrSem;

    /* ����ź����ǲ������ȼ������㷨 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* ��ȡ�������ز��� */
        ucTaskPrio = pstrTcb->ucTaskPrio;
        pstrChain = &pstrSem->strSemTab.astrChain[ucTaskPrio];
        pstrPrioFlag = &pstrSem->strSemTab.strFlag;

        /* ��sem���ȱ�ɾ����ǰ���� */
        return MDS_TaskDelFromSchedTab(pstrChain, pstrPrioFlag, ucTaskPrio);
    }
    else /* ����ź��������Ƚ��ȳ������㷨 */
    {
        /* ��ȡ�������ز���, ʹ��0���ȼ�������Ϊ�Ƚ��ֳ����� */
        pstrChain = &pstrSem->strSemTab.astrChain[LOWESTPRIO];

        /* ��sem���ȱ�ɾ����ǰ���� */
        return MDS_ChainNodeDelete(pstrChain);
    }
}

/***********************************************************************************
��������: �ڱ����ź��������������л�ȡһ����Ҫ�������ͷŵ�����.
��ڲ���: pstrSem: �ź���ָ��.
�� �� ֵ: ��Ҫ�ͷŵ������TCBָ��, ��û����Ҫ�ͷŵ������򷵻�NULL.
***********************************************************************************/
M_TCB* MDS_SemGetActiveTask(M_SEM* pstrSem)
{
    M_CHAIN* pstrNode;
    M_TCBQUE* pstrTaskQue;
    U8 ucTaskPrio;

    /* �ź����������ȼ������㷨 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* ��ȡ�ź����������ȼ���ߵ�����TCB */
        ucTaskPrio = MDS_TaskHighestPrioGet(&pstrSem->strSemTab.strFlag);
    }
    else /* �ź��������Ƚ��ȳ������㷨 */
    {
        /* ����0���ȼ������� */
        ucTaskPrio = LOWESTPRIO;
    }

    pstrNode = MDS_ChainEmpInq(&pstrSem->strSemTab.astrChain[ucTaskPrio]);

    /* �ź�����û�б����������� */
    if(NULL == pstrNode)
    {
        return (M_TCB*)NULL;
    }
    else
    {
        pstrTaskQue = (M_TCBQUE*)pstrNode;

        return pstrTaskQue->pstrTcb;
    }
}

/***********************************************************************************
��������: ������Ⱥ���, �������״̬ת���ĵ����ڴ˺���ʵ��.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSched(void)
{
    M_TCB* pstrTcb;

    /* ����delay������ */
    MDS_TaskDelayTabSched();

    /* ����ready������ */
    pstrTcb = MDS_TaskReadyTabSched();

#ifdef MDS_INCLUDETASKHOOK

    /* ��������л������Ѿ��ҽӺ�����ִ�иú��� */
    if((VFHSWT)NULL != gvfTaskSwitchHook)
    {
        /* ��ͬ����֮���л���ִ�������л����Ӻ��� */
        if(pstrTcb != gpstrCurTcb)
        {
            gvfTaskSwitchHook(gpstrCurTcb, pstrTcb);
        }
    }

#endif

    /* ׼�������л� */
    MDS_TaskSwitch(pstrTcb);
}

/***********************************************************************************
��������: ������Ⱥ���, ready̬����ĵ����ڴ˺���ʵ��.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskReadySched(void)
{
    M_TCB* pstrTcb;

    /* ����ready������ */
    pstrTcb = MDS_TaskReadyTabSched();

#ifdef MDS_INCLUDETASKHOOK

    /* ��������л������Ѿ��ҽӺ�����ִ�иú��� */
    if((VFHSWT)NULL != gvfTaskSwitchHook)
    {
        /* ��ͬ����֮���л���ִ�������л����Ӻ��� */
        if(pstrTcb != gpstrCurTcb)
        {
            gvfTaskSwitchHook(gpstrCurTcb, pstrTcb);
        }
    }

#endif

    /* ׼�������л� */
    MDS_TaskSwitch(pstrTcb);
}

/***********************************************************************************
��������: ��ready����в���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
M_TCB* MDS_TaskReadyTabSched(void)
{
    M_TCB* pstrTcb;
    M_TCBQUE* pstrTaskQue;
    U8 ucTaskPrio;

    /* ��ȡready�������ȼ���ߵ������TCB */
    ucTaskPrio = MDS_TaskHighestPrioGet(&gstrReadyTab.strFlag);
    pstrTaskQue = (M_TCBQUE*)MDS_ChainEmpInq(&gstrReadyTab.astrChain[ucTaskPrio]);
    pstrTcb = pstrTaskQue->pstrTcb;

    return pstrTcb;
}

/***********************************************************************************
��������: ��delay����в���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskDelayTabSched(void)
{
    M_TCB* pstrTcb;
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_CHAIN* pstrDelayNode;
    M_CHAIN* pstrNextNode;
    M_PRIOFLAG* pstrPrioFlag;
    M_TCBQUE* pstrDelayQue;
    U32 uiTick;
    U8 ucTaskPrio;

    /* ��ȡdelay���е�����ڵ� */
    pstrDelayNode = MDS_ChainEmpInq(&gstrDelayTab);

    /* delay����������, ����delay���е����� */
    if(NULL != pstrDelayNode)
    {
        /* �ж�delay����������ӳ�ʱ���Ƿ���� */
        while(1)
        {
            /* ��ȡdelay���е�������ӳ�ʱ�� */
            pstrDelayQue = (M_TCBQUE*)pstrDelayNode;
            pstrTcb = pstrDelayQue->pstrTcb;
            uiTick = pstrTcb->uiStillTick;

            /* �������ӳ�ʱ�䵽, ��delay����ɾ�������뵽���ȱ��� */
            if(uiTick == guiTick)
            {
                /* ��delay���������� */
                pstrNextNode = MDS_ChainCurNodeDelete(&gstrDelayTab, pstrDelayNode);

                /* ��������delay���־ */
                pstrTcb->uiTaskFlag &= (~((U32)DELAYQUEFLAG));

                /* ���¸������delay��pend��ز��� */
                /* �������ӵ��delay״̬���delay״̬�ָ� */
                if(TASKDELAY == (TASKDELAY & pstrTcb->strTaskOpt.ucTaskSta))
                {
                    /* ��������delay״̬ */
                    pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKDELAY);

                    /* ����uiDelayTick��������delay����ķ���ֵ */
                    pstrTcb->strTaskOpt.uiDelayTick = RTN_TKDLTO;
                }
                /* �������ӵ��pend״̬���pend״̬�ָ� */
                else if(TASKPEND == (TASKPEND& pstrTcb->strTaskOpt.ucTaskSta))
                {
                    /* ���ź������ȱ������� */
                    (void)MDS_TaskDelFromSemTab(gpstrCurTcb);

                    /* ��������pend״̬ */
                    pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKPEND);

                    /* ����uiDelayTick��������pend����ķ���ֵ */
                    pstrTcb->strTaskOpt.uiDelayTick = RTN_SMTKTO;
                }

                /* ��ȡ���������ز��� */
                pstrNode = &pstrTcb->strTcbQue.strQueHead;
                ucTaskPrio = pstrTcb->ucTaskPrio;
                pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
                pstrPrioFlag = &gstrReadyTab.strFlag;

                /* ����������ӵ�ready���� */
                MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag,
                                      ucTaskPrio);

                /* ���������ready״̬ */
                pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;

                /* delay���Ѿ��������, ������delay��ĵ��� */
                if(NULL == pstrNextNode)
                {
                    break;
                }
                else /* delay��û������, �����¸��ڵ�����ж� */
                {
                    pstrDelayNode = pstrNextNode;
                }
            }
            else /* ��������û�е�ʱ��, ������delay��ĵ��� */
            {
                break;
            }
        }
    }
}

/***********************************************************************************
��������: ����������ӵ�������ȱ��Ӧ�����ȼ���־����.
��ڲ���: pstrPrioFlag: ���ȱ��Ӧ�����ȼ���־��ָ��.
          ucTaskPrio: ��������ȼ�.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskPrioFlagSet(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
    U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
#endif

    /* ���õ��ȱ��Ӧ�����ȼ���־�� */
#if PRIORITYNUM >= PRIORITY128

    /* ��ȡ���ȼ���־�ڵ�һ��͵ڶ����е���� */
    ucPrioFlagGrp1 = ucTaskPrio / 8;
    ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;

    /* ��ȡ���ȼ���־��ÿһ���е�λ�� */
    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1 % 8;
    ucPosInGrp3 = ucPrioFlagGrp2;

    /* ��ÿһ�����������ȼ���־ */
    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] |= (U8)(1 << ucPosInGrp1);
    pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] |= (U8)(1 << ucPosInGrp2);
    pstrPrioFlag->ucPrioFlagGrp3 |= (U8)(1 << ucPosInGrp3);

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = ucTaskPrio / 8;

    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1;

    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] |= (U8)(1 << ucPosInGrp1);
    pstrPrioFlag->ucPrioFlagGrp2 |= (U8)(1 << ucPosInGrp2);

#else

    pstrPrioFlag->ucPrioFlagGrp1 |= (U8)(1 << ucTaskPrio);

#endif
}

/***********************************************************************************
��������: ���������������ȱ��Ӧ�����ȼ���־�������.
��ڲ���: pstrPrioFlag: ���ȱ��Ӧ�����ȼ���־��ָ��.
          ucTaskPrio: ��������ȼ�.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskPrioFlagClr(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
    U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
#endif

    /* ������ȱ��Ӧ�����ȼ���־�� */
#if PRIORITYNUM >= PRIORITY128

    /* ��ȡ���ȼ���־�ڵ�һ��͵ڶ����е���� */
    ucPrioFlagGrp1 = ucTaskPrio / 8;
    ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;

    /* ��ȡ���ȼ���־��ÿһ���е�λ�� */
    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1 % 8;
    ucPosInGrp3 = ucPrioFlagGrp2;

    /* ��ÿһ����������ȼ���־ */
    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &= ~((U8)(1 << ucPosInGrp1));
    if(0 == pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1])
    {
        pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] &= ~((U8)(1 << ucPosInGrp2));
        if(0 == pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2])
        {
            pstrPrioFlag->ucPrioFlagGrp3 &= ~((U8)(1 << ucPosInGrp3));
        }
    }

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = ucTaskPrio / 8;

    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1;

    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &= ~((U8)(1 << ucPosInGrp1));
    if(0 == pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1])
    {
        pstrPrioFlag->ucPrioFlagGrp2 &= ~((U8)(1 << ucPosInGrp2));
    }

#else

    pstrPrioFlag->ucPrioFlagGrp1 &= ~((U8)(1 << ucTaskPrio));

#endif
}

/***********************************************************************************
��������: ��ȡ������ȱ��������������ȼ�.
��ڲ���: pstrPrioFlag: ���ȱ�����ȼ���־��ָ��.
�� �� ֵ: ������ȱ��е�������ȼ�.
***********************************************************************************/
U8 MDS_TaskHighestPrioGet(M_PRIOFLAG* pstrPrioFlag)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucHighestFlagInGrp1;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucHighestFlagInGrp1;
#endif

    /* ��ȡ������ȱ��е�������ȼ� */
#if PRIORITYNUM >= PRIORITY128

    ucPrioFlagGrp2 = caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp3];

    ucPrioFlagGrp1 =
                caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2]];

    ucHighestFlagInGrp1 = caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp1
                                             [ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1]];

    return (U8)((ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1) * 8 + ucHighestFlagInGrp1);

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp2];

    ucHighestFlagInGrp1 =
                caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1]];

    return (U8)(ucPrioFlagGrp1 * 8 + ucHighestFlagInGrp1);

#else

    return caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp1];

#endif
}

/***********************************************************************************
��������: ��ȡϵͳ��ǰ��tick.
��ڲ���: none.
�� �� ֵ: ϵͳ��ǰ��tick.
***********************************************************************************/
U32 MDS_SystemTickGet(void)
{
    return guiTick;
}

/***********************************************************************************
��������: ��ȡϵͳ��ǰ�����TCB.
��ڲ���: none.
�� �� ֵ: ϵͳ��ǰ��TCB.
***********************************************************************************/
M_TCB* MDS_CurrentTcbGet(void)
{
    return gpstrCurTcb;
}

/***********************************************************************************
��������: ��ֹ���ж�, ��userģʽ�µ��øú��������κβ���. ������MDS_IntUnlock������
          ��ʹ��.
��ڲ���: none.
�� �� ֵ: RTN_SUCD: ���жϳɹ�.
          RTN_FAIL: ���ж�ʧ��.
***********************************************************************************/
U32 MDS_IntLock(void)
{
    /* �ǲ���ϵͳ״̬, �������ж� */
    if(NULL == gpstrCurTcb)
    {
        return RTN_SUCD;
    }

    /* ������ж������иú�����ֱ�ӷ��� */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return RTN_SUCD;
    }

    /* ��һ�ε��øú�������ʵ�ʵ����жϲ��� */
    if(0 == guiIntLockCounter)
    {
        MDS_TaskOccurSwi(SWI_INTDISABLE);

        guiIntLockCounter++;

        return RTN_SUCD;
    }
    /* �ǵ�һ�ε��øú�������С����������ֱ�ӷ��سɹ� */
    else if(guiIntLockCounter < 0xFFFFFFFF)
    {
        guiIntLockCounter++;

        return RTN_SUCD;
    }
    else /* ������������ֱ�ӷ���ʧ�� */
    {
        return RTN_FAIL;
    }
}

/***********************************************************************************
��������: ʹ�����ж�, ��userģʽ�µ��øú��������κβ���. ������MDS_IntLock�����ɶ�
          ʹ��.
��ڲ���: none.
�� �� ֵ: RTN_SUCD: �����жϳɹ�.
          RTN_FAIL: �����ж�ʧ��.
***********************************************************************************/
U32 MDS_IntUnlock(void)
{
    /* �ǲ���ϵͳ״̬, �������ж� */
    if(NULL == gpstrCurTcb)
    {
        return RTN_SUCD;
    }

    /* ������ж������иú�����ֱ�ӷ��� */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return RTN_SUCD;
    }

    /* �ǵ�һ�ε��øú���ֱ�ӷ��سɹ� */
    if(guiIntLockCounter > 1)
    {
        guiIntLockCounter--;

        return RTN_SUCD;
    }
    /* ���һ�ε��øú�������ʵ�ʵĽ����жϲ��� */
    else if(1 == guiIntLockCounter)
    {
        guiIntLockCounter--;

        MDS_TaskOccurSwi(SWI_INTENABLE);

        return RTN_SUCD;
    }
    else /* ����0����ֱ�ӷ���ʧ�� */
    {
        return RTN_FAIL;
    }
}

/***********************************************************************************
��������: ��ʼ������.
��ڲ���: pstrChain: ������ڵ�ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_ChainInit(M_CHAIN* pstrChain)
{
    /* �������ͷβ��ָ��սڵ� */
    pstrChain->pstrHead = (M_CHAIN*)NULL;
    pstrChain->pstrTail = (M_CHAIN*)NULL;
}

/***********************************************************************************
��������: ���������һ���ڵ�, ������β������.
��ڲ���: pstrChain: ������ڵ�ָ��.
          pstrNode: ����Ľڵ�ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_ChainNodeAdd(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* ����ǿ� */
    if(NULL != pstrChain->pstrTail)
    {
        /* �½ڵ��ͷָ��ԭβ�ڵ� */
        pstrNode->pstrHead = pstrChain->pstrHead;

        /* �½ڵ��βָ����ڵ� */
        pstrNode->pstrTail = pstrChain;

        /* ԭβ�ڵ��βָ���½ڵ� */
        pstrChain->pstrHead->pstrTail = pstrNode;

        /* ���ڵ��ͷָ���¼���Ľڵ� */
        pstrChain->pstrHead = pstrNode;
    }
    else /* ����Ϊ�� */
    {
        /* �½ڵ��ͷβ��ָ����ڵ� */
        pstrNode->pstrHead = pstrChain;
        pstrNode->pstrTail = pstrChain;

        /* ���ڵ��ͷβ��ָ���½ڵ� */
        pstrChain->pstrHead = pstrNode;
        pstrChain->pstrTail = pstrNode;
    }
}

/***********************************************************************************
��������: ������ɾ��һ���ڵ�, ������ͷ��ɾ��.
��ڲ���: pstrChain: ������ڵ�ָ��.
�� �� ֵ: ɾ���Ľڵ�ָ��, ������Ϊ���򷵻�NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainNodeDelete(M_CHAIN* pstrChain)
{
    M_CHAIN* pstrTempNode;

    /* �����еĵ�һ���ڵ� */
    pstrTempNode = pstrChain->pstrTail;

    /* ����ǿ� */
    if(NULL != pstrTempNode)
    {
        /* �������ж���ڵ� */
        if(pstrChain->pstrHead != pstrChain->pstrTail)
        {
            /* ���ڵ��βָ��ڶ����ڵ� */
            pstrChain->pstrTail = pstrTempNode->pstrTail;

            /* �ڶ����ڵ��ͷָ����ڵ� */
            pstrTempNode->pstrTail->pstrHead = pstrChain;
        }
        else /* ������ֻ��һ���ڵ� */
        {
            /* ȡ���ڵ������Ϊ�� */
            pstrChain->pstrHead = (M_CHAIN*)NULL;
            pstrChain->pstrTail = (M_CHAIN*)NULL;
        }

        /* ����ȡ���Ľڵ�ָ�� */
        return pstrTempNode;
    }
    else /* ����Ϊ�շ���NULL */
    {
        return (M_CHAIN*)NULL;
    }
}

/***********************************************************************************
��������: ������ָ���Ľڵ�ǰ����һ���ڵ�.
��ڲ���: pstrChain: ������ڵ�ָ��.
          pstrNode: ��׼�ڵ�ָ��, ���½ڵ�嵽�ýڵ�ǰ��.
          pstrNewNode: �²���ڵ��ָ��.
�� �� ֵ: none.
***********************************************************************************/
void MDS_ChainCurNodeInsert(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                            M_CHAIN* pstrNewNode)
{
    /* ��׼�ڵ㲻�Ǹ��ڵ� */
    if(pstrChain != pstrNode)
    {
        /* ��׼�ڵ���ϸ��ڵ��βָ���½ڵ� */
        pstrNode->pstrHead->pstrTail = pstrNewNode;

        /* �½ڵ��ͷָ���׼�ڵ���ϸ��ڵ� */
        pstrNewNode->pstrHead = pstrNode->pstrHead;

        /* �½ڵ��βָ���׼�ڵ� */
        pstrNewNode->pstrTail = pstrNode;

        /* ��׼�ڵ��ͷָ���½ڵ� */
        pstrNode->pstrHead = pstrNewNode;
    }
    else /* ��׼�ڵ��Ǹ��ڵ� */
    {
        MDS_ChainNodeAdd(pstrChain, pstrNewNode);
    }
}

/***********************************************************************************
��������: ������ɾ��ָ���Ľڵ�, �������¸��ڵ��ָ��.
��ڲ���: pstrChain: ������ڵ�ָ��.
          pstrNode: Ҫɾ���Ľڵ��ָ��.
�� �� ֵ: ɾ���ڵ���¸��ڵ�ָ��, ��û���¸��ڵ��򷵻�NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainCurNodeDelete(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* Ҫɾ���Ľڵ㲻�Ǹ��ڵ� */
    if(pstrChain != pstrNode)
    {
        /* �������ж���ڵ� */
        if((pstrNode->pstrHead != pstrChain) || (pstrNode->pstrTail != pstrChain))
        {
            /* Ҫɾ���ڵ���ϸ��ڵ��βָ��Ҫɾ���ڵ���¸��ڵ� */
            pstrNode->pstrHead->pstrTail = pstrNode->pstrTail;

            /* Ҫɾ���ڵ���¸��ڵ��ͷָ��Ҫɾ���ڵ���ϸ��ڵ� */
            pstrNode->pstrTail->pstrHead = pstrNode->pstrHead;

            /* ����ɾ���ڵ���¸��ڵ�ָ�� */
            return pstrNode->pstrTail;
        }
        else /* ������ֻ��һ���ڵ� */
        {
            (void)MDS_ChainNodeDelete(pstrChain);

            /* û���¸��ڵ�, ����NULL */
            return (M_CHAIN*)NULL;
        }
    }
    else /* ɾ�����ڵ�ֱ�ӷ���NULL */
    {
        return (M_CHAIN*)NULL;
    }
}

/***********************************************************************************
��������: ��ѯ�����Ƿ�Ϊ��.
��ڲ���: pstrChain: ������ڵ�ָ��.
�� �� ֵ: ���ǿ��򷵻ص�һ���ڵ��ָ��, �����򷵻�NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainEmpInq(M_CHAIN* pstrChain)
{
    return pstrChain->pstrTail;
}

/***********************************************************************************
��������: ��ѯ������ָ���ڵ����һ���ڵ��Ƿ�Ϊ��.
��ڲ���: pstrChain: ������ڵ�ָ��.
          pstrNode: ��׼�ڵ�ָ��, ��ѯ�ýڵ����һ���ڵ�.
�� �� ֵ: ��ָ���ڵ����һ���ڵ�ǿ��򷵻���һ���ڵ��ָ��, �����򷵻�NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainNextNodeEmpInq(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* ��׼�ڵ������һ���ڵ��򷵻�NULL */
    if(pstrNode->pstrTail == pstrChain)
    {
        return (M_CHAIN*)NULL;
    }
    else
    {
        return pstrNode->pstrTail;
    }
}

