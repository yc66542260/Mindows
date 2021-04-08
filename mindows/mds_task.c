
#include "mindows_inner.h"


#ifdef MDS_INCLUDETASKHOOK
VFHCRT gvfTaskCreateHook;               /* ���񴴽����ӱ��� */
VFHSWT gvfTaskSwitchHook;               /* �����л����ӱ��� */
VFHDLT gvfTaskDeleteHook;               /* ����ɾ�����ӱ��� */
#endif


/***********************************************************************************
��������: ����һ������.
��ڲ���: pucTaskName: ָ���������Ƶ�ָ��.
          vfFuncPointer: ����������ʹ�ú�����ָ��.
          pucTaskStack: ������ʹ�ö�ջ�������ʼ��ַ. ��Ϊ��, ����������������.
          uiStackSize: ��ջ��С, ��λ: �ֽ�.
          ucTaskPrio: �������ȼ�.
          pstrTaskOpt: �������ָ��.
�� �� ֵ: NULL: ��������ʧ��.
          ����: �����TCBָ��.
***********************************************************************************/
M_TCB* MDS_TaskCreate(U8* pucTaskName, VFUNC vfFuncPointer, U8* pucTaskStack,
                      U32 uiStackSize, U8 ucTaskPrio, M_TASKOPT* pstrTaskOpt)
{
    M_TCB* pstrTcb;

    /* pucTaskName��pstrTaskOpt�������Ǳ���, ������� */

    /* �Դ���������ʹ�ú�����ָ��Ϸ��Խ��м�� */
    if(NULL == vfFuncPointer)
    {
        /* ָ��Ϊ��, ����ʧ�� */
        return (M_TCB*)NULL;
    }

    /* �������ջ�Ϸ��Խ��м�� */
    if((NULL == pucTaskStack) || (0 == uiStackSize))
    {
        /* ��ջ���Ϸ�, ����ʧ�� */
        return (M_TCB*)NULL;
    }

    /* �����������ʱ������״̬�Ϸ��Խ��м�� */
    if(NULL != pstrTaskOpt)
    {
        /* �����ڵ�����״̬����ʧ�� */
        if(!((TASKREADY == pstrTaskOpt->ucTaskSta)
             || (TASKDELAY == pstrTaskOpt->ucTaskSta)))
        {
            return (M_TCB*)NULL;
        }
    }

    /* �Է�ϵͳ�������ȼ���� */
    if((MDS_RootTask != vfFuncPointer) && (MDS_IdleTask != vfFuncPointer))
    {
        /* ��ϵͳ�������ȼ�������������ȼ�, Ҳ���ܵ���������ȼ� */
        if((HIGHESTPRIO == ucTaskPrio) || (ucTaskPrio >= LOWESTPRIO))
        {
            return (M_TCB*)NULL;
        }
    }

    /* ���Ƿ��ظ�����root������м�� */
    if(MDS_RootTask == vfFuncPointer)
    {
        /* root�����Ѿ�������, ����ʧ�� */
        if(NULL != gpstrRootTaskTcb)
        {
            return (M_TCB*)NULL;
        }
    }

    /* ���Ƿ��ظ�����idle������м�� */
    if(MDS_IdleTask == vfFuncPointer)
    {
        /* idle�����Ѿ�������, ����ʧ�� */
        if(NULL != gpstrIdleTaskTcb)
        {
            return (M_TCB*)NULL;
        }
    }

    /* ��ʼ��TCB */
    pstrTcb = MDS_TaskTcbInit(pucTaskName, vfFuncPointer, pucTaskStack, uiStackSize,
                              ucTaskPrio, pstrTaskOpt);

    /* ������ϵͳ�����Ĳ��� */
    if((MDS_RootTask != vfFuncPointer) && (MDS_IdleTask != vfFuncPointer))
    {
#ifdef MDS_INCLUDETASKHOOK

        /* ������񴴽������Ѿ��ҽӺ�����ִ�иú��� */
        if((VFHCRT)NULL != gvfTaskCreateHook)
        {
            gvfTaskCreateHook(pstrTcb);
        }

#endif

        /* ʹ�����жϵ������� */
        MDS_TaskSwiSched();
    }

    return pstrTcb;
}

/***********************************************************************************
��������: ɾ��һ������.
��ڲ���: pstrTcb: Ҫɾ���������TCBָ��.
�� �� ֵ: RTN_SUCD: ����ɾ���ɹ�.
          RTN_FAIL: ����ɾ��ʧ��.
***********************************************************************************/
U32 MDS_TaskDelete(M_TCB* pstrTcb)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;
    U8 ucTaskSta;

    /* ��ڲ������ */
    if(NULL == pstrTcb)
    {
        return RTN_FAIL;
    }

    /* idle�����ܱ�ɾ�� */
    if(pstrTcb == gpstrIdleTaskTcb)
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* ��ȡҪɾ�����������״̬ */
    ucTaskSta = pstrTcb->strTaskOpt.ucTaskSta;

    /* ��������ready����, ��ready��ɾ�� */
    if(TASKREADY == (TASKREADY & ucTaskSta))
    {
        /* ��ȡ���������ص��Ȳ��� */
        ucTaskPrio = pstrTcb->ucTaskPrio;
        pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
        pstrPrioFlag = &gstrReadyTab.strFlag;

        /* ���������ready���� */
        (void)MDS_TaskDelFromSchedTab(pstrChain, pstrPrioFlag, ucTaskPrio);
    }

    /* ������delay�����delay���� */
    if(DELAYQUEFLAG == (pstrTcb->uiTaskFlag & DELAYQUEFLAG))
    {
        /* ��ȡ������TCB�йҽ���delay���ȱ��ϵĽڵ� */
        pstrNode = &pstrTcb->strDelayQue.strQueHead;

        /* ��delay���������� */
        (void)MDS_ChainCurNodeDelete(&gstrDelayTab, pstrNode);
    }

    /* ����ӵ��pend״̬ */
    if(TASKPEND == (TASKPEND & ucTaskSta))
    {
        /* ���ź������ȱ������� */
        (void)MDS_TaskDelFromSemTab(pstrTcb);
    }

    (void)MDS_IntUnlock();

    /* �������ɾ�������Ѿ��ҽӺ�����ִ�иú��� */
    if((VFHDLT)NULL != gvfTaskDeleteHook)
    {
        gvfTaskDeleteHook(pstrTcb);
    }

    /* ʹ�����жϵ������� */
    MDS_TaskSwiSched();

    return RTN_SUCD;
}

/***********************************************************************************
��������: ɾ����ǰ���е�����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSelfDelete(void)
{
    (void)MDS_TaskDelete(gpstrCurTcb);
}

/***********************************************************************************
��������: ��ʼ������TCB, ����������������ǰTCB��״̬.
��ڲ���: pucTaskName: ָ���������Ƶ�ָ��.
          vfFuncPionter: ����������ʹ�õĺ���ָ��.
          pucTaskStack: ������ʹ�õĶ�ջ��ַ.
          uiStackSize: ��ջ��С, ��λ: �ֽ�.
          ucTaskPrio: �������ȼ�.
          pstrTaskOpt: �������ָ��.
�� �� ֵ: NULL: ��������ʧ��.
          ����: �����TCBָ��.
***********************************************************************************/
M_TCB* MDS_TaskTcbInit(U8* pucTaskName, VFUNC vfFuncPointer, U8* pucTaskStack,
                       U32 uiStackSize, U8 ucTaskPrio, M_TASKOPT* pstrTaskOpt)
{
    M_TCB* pstrTcb;
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8* pucStackBy4;

    /* ��ջ����ַ, ��Ҫ4�ֽڶ��� */
    pucStackBy4 = (U8*)(((U32)pucTaskStack + uiStackSize) & 0xFFFFFFFC);

    /* TCB�ṹ��ŵĵ�ַ, ��Ҫ4�ֽڶ��� */
    pstrTcb = (M_TCB*)(((U32)pucStackBy4 - sizeof(M_TCB)) & 0xFFFFFFFC);

    /* ��ʼ�������ջ */
    MDS_TaskStackInit(pstrTcb, vfFuncPointer);

    /* �Ƚ������־��ʼ��Ϊȫ��, ������Ϊ�����Ӿ���Ĺ��ܱ�־ */
    pstrTcb->uiTaskFlag = 0;

    /* ��ʼ��ָ��TCB��ָ�� */
    pstrTcb->strTcbQue.pstrTcb = pstrTcb;
    pstrTcb->strDelayQue.pstrTcb = pstrTcb;

    /* ��ʼ��ָ���������Ƶ�ָ�� */
    pstrTcb->pucTaskName = pucTaskName;

    /* ��ʼ���������ȼ� */
    pstrTcb->ucTaskPrio = ucTaskPrio;

    /* û���������������״̬����Ϊready̬ */
    if(NULL == pstrTaskOpt)
    {
        pstrTcb->strTaskOpt.ucTaskSta = TASKREADY;
    }
    else /* ����������򽫲������Ƶ�TCB�� */
    {
        pstrTcb->strTaskOpt.ucTaskSta = pstrTaskOpt->ucTaskSta;
        pstrTcb->strTaskOpt.uiDelayTick = pstrTaskOpt->uiDelayTick;
    }

    /* ���ж�, ��ֹ��������Ӱ�� */
    (void)MDS_IntLock();

    /* �������������ready̬, ���������ready�� */
    if(TASKREADY == (TASKREADY & pstrTcb->strTaskOpt.ucTaskSta))
    {
        pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
        pstrNode = &pstrTcb->strTcbQue.strQueHead;
        pstrPrioFlag = &gstrReadyTab.strFlag;

        /* ����������ӵ�ready���� */
        MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);
    }

    /* �������������delay̬, ���������delay�� */
    if(TASKDELAY == (TASKDELAY & pstrTcb->strTaskOpt.ucTaskSta))
    {
        /* �����õȴ�����Ź���delay�� */
        if(DELAYWAITFEV != pstrTaskOpt->uiDelayTick)
        {
            /* �����½�������ӳ�ʱ�� */
            pstrTcb->uiStillTick = guiTick + pstrTaskOpt->uiDelayTick;

            /* ������������ȡdelay��ڵ㲢���뵽delay�� */
            pstrNode = &pstrTcb->strDelayQue.strQueHead;
            MDS_TaskAddToDelayTab(pstrNode);

            /* ��������delay���־ */
            pstrTcb->uiTaskFlag |= DELAYQUEFLAG;
        }
    }

    /* �������������ж�, ����������� */
    (void)MDS_IntUnlock();

    return pstrTcb;
}

/***********************************************************************************
��������: �������delay̬, �ӳ�ָ����ʱ��. ���ô˺������������л�.
��ڲ���: uiDelaytick: ������Ҫ�ӳٵ�ʱ��, ��λ: tick.
                       DELAYNOWAIT: ���ȴ�, �����������л�.
                       DELAYWAITFEV: ����һֱ�ȴ�.
                       ����: ������Ҫ�ӳٵ�tick��Ŀ.
�� �� ֵ: RTN_SUCD: �����ӳٳɹ�.
          RTN_FAIL: �����ӳ�ʧ��.
          RTN_TKDLTO: �����ӳ�ʱ��ľ�, ��ʱ����.
          RTN_TKDLBK: �����ӳ�״̬���ж�, ���񷵻�.
***********************************************************************************/
U32 MDS_TaskDelay(U32 uiDelayTick)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_CHAIN* pstrDelayNode;
    M_TCBQUE* pstrTaskQue;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* idle�����ܴ���delay״̬ */
    if(gpstrCurTcb == gpstrIdleTaskTcb)
    {
        return RTN_FAIL;
    }

    /* �ӳ�ʱ�䲻Ϊ0 tick���������, �����ӳ�ֱ���л����� */
    if(DELAYNOWAIT != uiDelayTick)
    {
        /* ��ȡ��ǰ�������ص��Ȳ��� */
        ucTaskPrio = gpstrCurTcb->ucTaskPrio;
        pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
        pstrPrioFlag = &gstrReadyTab.strFlag;

        (void)MDS_IntLock();

        /* ����ǰ�����ready���� */
        pstrNode = MDS_TaskDelFromSchedTab(pstrChain, pstrPrioFlag, ucTaskPrio);

        /* ��������ready״̬ */
        gpstrCurTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKREADY);

        /* ���µ�ǰ������ӳ�ʱ�� */
        gpstrCurTcb->strTaskOpt.uiDelayTick = uiDelayTick;

        /* �����õȴ�����Ź���delay�� */
        if(DELAYWAITFEV != uiDelayTick)
        {
            gpstrCurTcb->uiStillTick = guiTick + uiDelayTick;

            /* ��ȡ��ǰ����delay�������ڵ� */
            pstrTaskQue = (M_TCBQUE*)pstrNode;
            pstrDelayNode = &pstrTaskQue->pstrTcb->strDelayQue.strQueHead;

            /* ����ǰ������뵽delay�� */
            MDS_TaskAddToDelayTab(pstrDelayNode);

            /* ��������delay���־ */
            gpstrCurTcb->uiTaskFlag |= DELAYQUEFLAG;
        }

        /* ���������delay״̬ */
        gpstrCurTcb->strTaskOpt.ucTaskSta |= TASKDELAY;

        (void)MDS_IntUnlock();
    }
    else /* �����ӳ�, �����������л� */
    {
        /* ����uiDelayTick���������ӳ�����ķ���ֵ */
        gpstrCurTcb->strTaskOpt.uiDelayTick = RTN_SUCD;
    }

    /* ʹ�����жϵ������� */
    MDS_TaskSwiSched();

    /* �����ӳ�����ķ���ֵ, �����delay״̬����ʱ����ֵ��������uiDelayTick�� */
    return gpstrCurTcb->strTaskOpt.uiDelayTick;
}

/***********************************************************************************
��������: ������ֻ�ܻ��������delay״̬.
��ڲ���: pstrTcb: �����������TCBָ��.
�� �� ֵ: RTN_SUCD: �����Ѳ����ɹ�.
          RTN_FAIL: �����Ѳ���ʧ��.
***********************************************************************************/
U32 MDS_TaskWake(M_TCB* pstrTcb)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* ��ڲ������ */
    if(NULL == pstrTcb)
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* �����Ի��������delay״̬ */
    if(TASKDELAY != (TASKDELAY & pstrTcb->strTaskOpt.ucTaskSta))
    {
        (void)MDS_IntUnlock();

        return RTN_FAIL;
    }

    pstrNode = &pstrTcb->strDelayQue.strQueHead;

    /* �����õȴ�����Ŵ�delay���� */
    if(DELAYWAITFEV != pstrTcb->strTaskOpt.uiDelayTick)
    {
        /* ��delay���������� */
        (void)MDS_ChainCurNodeDelete(&gstrDelayTab, pstrNode);

        /* ��������delay���־ */
        pstrTcb->uiTaskFlag &= (~((U32)DELAYQUEFLAG));
    }

    /* ��������delay״̬ */
    pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKDELAY);

    /* ����uiDelayTick���������ӳ�����ķ���ֵ */
    pstrTcb->strTaskOpt.uiDelayTick = RTN_TKDLBK;

    /* ��ȡ���������ز��� */
    ucTaskPrio = pstrTcb->ucTaskPrio;
    pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
    pstrPrioFlag = &gstrReadyTab.strFlag;

    /* ����������ӵ�ready���� */
    MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);

    /* ���������ready״̬ */
    pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;

    (void)MDS_IntUnlock();

    /* ʹ�����жϵ������� */
    MDS_TaskSwiSched();

    return RTN_SUCD;
}

/***********************************************************************************
��������: �������pend̬, �ӳ�ָ����ʱ��. ���ô˺������������л�.
��ڲ���: pstrSem: ����������ź���ָ��.
          uiDelaytick: ������Ҫ�ӳٵ�ʱ��, ��λ: tick.
                       DELAYNOWAIT: ���ȴ�, �����������л�.
                       DELAYWAITFEV: ����һֱ�ȴ�.
                       ����: ������Ҫ�ӳٵ�tick��Ŀ.
�� �� ֵ: RTN_SUCD: ���������ɹ�.
          RTN_FAIL: ��������ʧ��..
***********************************************************************************/
U32 MDS_TaskPend(M_SEM* pstrSem, U32 uiDelayTick)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_CHAIN* pstrDelayNode;
    M_TCBQUE* pstrTaskQue;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* idle�����ܴ���delay״̬ */
    if(gpstrCurTcb == gpstrIdleTaskTcb)
    {
        return RTN_FAIL;
    }

    /* ��ȡ��ǰ�������ص��Ȳ��� */
    ucTaskPrio = gpstrCurTcb->ucTaskPrio;
    pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
    pstrPrioFlag = &gstrReadyTab.strFlag;

    /* ����ǰ�����ready���� */
    pstrNode = MDS_TaskDelFromSchedTab(pstrChain, pstrPrioFlag, ucTaskPrio);

    /* ��������ready״̬ */
    gpstrCurTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKREADY);

    /* ���µ�ǰ������ӳ�ʱ�� */
    gpstrCurTcb->strTaskOpt.uiDelayTick = uiDelayTick;

    /* �����õȴ�����Ź���delay�� */
    if(SEMWAITFEV != uiDelayTick)
    {
        gpstrCurTcb->uiStillTick = guiTick + uiDelayTick;

        /* ��ȡ��ǰ����delay�������ڵ� */
        pstrTaskQue = (M_TCBQUE*)pstrNode;
        pstrDelayNode = &pstrTaskQue->pstrTcb->strDelayQue.strQueHead;

        /* ����ǰ������뵽delay�� */
        MDS_TaskAddToDelayTab(pstrDelayNode);

        /* ��������delay���־ */
        gpstrCurTcb->uiTaskFlag |= DELAYQUEFLAG;
    }

    /* ����������ӵ��ź������ȱ��� */
    MDS_TaskAddToSemTab(gpstrCurTcb, pstrSem);

    /* ���������pend״̬ */
    gpstrCurTcb->strTaskOpt.ucTaskSta |= TASKPEND;

    return RTN_SUCD;
}

#ifdef MDS_INCLUDETASKHOOK

/***********************************************************************************
��������: ��ʼ�����Ӻ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskHookInit(void)
{
    /* ��ʼ�����ӱ��� */
    gvfTaskCreateHook = (VFHCRT)NULL;
    gvfTaskSwitchHook = (VFHSWT)NULL;
    gvfTaskDeleteHook = (VFHDLT)NULL;
}

/***********************************************************************************
��������: ������񴴽����Ӻ���.
��ڲ���: vfFuncPointer: ��ӵĹ��Ӻ���.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskCreateHookAdd(VFHCRT vfFuncPointer)
{
    gvfTaskCreateHook = vfFuncPointer;
}

/***********************************************************************************
��������: ɾ�����񴴽����Ӻ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskCreateHookDel(void)
{
    gvfTaskCreateHook = (VFHCRT)NULL;
}

/***********************************************************************************
��������: ��������л����Ӻ���.
��ڲ���: vfFuncPointer: ��ӵĹ��Ӻ���.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSwitchHookAdd(VFHSWT vfFuncPointer)
{
    gvfTaskSwitchHook = vfFuncPointer;
}

/***********************************************************************************
��������: ɾ�������л����Ӻ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSwitchHookDel(void)
{
    gvfTaskSwitchHook = (VFHSWT)NULL;
}

/***********************************************************************************
��������: �������ɾ�����Ӻ���.
��ڲ���: vfFuncPointer: ��ӵĹ��Ӻ���.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskDeleteHookAdd(VFHDLT vfFuncPointer)
{
    gvfTaskDeleteHook = vfFuncPointer;
}

/***********************************************************************************
��������: ɾ������ɾ�����Ӻ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskDeleteHookDel(void)
{
    gvfTaskDeleteHook = (VFHDLT)NULL;
}

#endif

