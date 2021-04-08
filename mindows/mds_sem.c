
#include "mindows_inner.h"


/***********************************************************************************
��������: ����һ���ź���.
��ڲ���: pstrSem: �ź�����ʹ�õ��ڴ�ָ��.
          uiSemOpt: �����ź������õ�ѡ��.
                    ���ȼ�:
                    SEMPRIO: �ź������в������ȼ�����, �ͷ��ź���ʱ�ȼ���pend������
                             ���ȼ���ߵ�����.
                    SEMFIFO: �ź������в����Ƚ��ȳ�����, �ͷ��ź���ʱ�ȼ������ȱ�
                             pend������.
                    �ź�������:
                    SEMBIN: �����������ź���.
                    SEMCNT: ���������ź���.
          uiInitVal: �ź����ĳ�ʼֵ.
                     SEMEMPTY: �ź���Ϊ��״̬, ���ɻ�ȡ.
                     SEMFULL: �ź���Ϊ��״̬, �ɻ�ȡ.
�� �� ֵ: RTN_SUCD: �����ź����ɹ�.
          RTN_FAIL: �����ź���ʧ��.
***********************************************************************************/
U32 MDS_SemCreate(M_SEM* pstrSem, U32 uiSemOpt, U32 uiInitVal)
{
    /* ��ڲ������ */
    if(NULL == pstrSem)
    {
        return RTN_FAIL;
    }

    /* �ź���ѡ���� */
    if(((SEMBIN != (SEMTYPEMASK & uiSemOpt))
        && (SEMCNT != (SEMTYPEMASK & uiSemOpt)))
       || ((SEMFIFO != (SEMSCHEDOPTMASK & uiSemOpt))
           && (SEMPRIO != (SEMSCHEDOPTMASK & uiSemOpt))))
    {
        return RTN_FAIL;
    }

    /* �������ź�����ʼֵֻ���ǿջ����� */
    if(SEMBIN == (SEMTYPEMASK & uiSemOpt))
    {
        if((SEMEMPTY != uiInitVal) && (SEMFULL != uiInitVal))
        {
            return RTN_FAIL;
        }
    }

    /* ��ʼ���ź������ȱ� */
    MDS_TaskSchedTabInit(&pstrSem->strSemTab);

    /* ��ʼ���ź�����ʼֵ */
    pstrSem->uiCounter = uiInitVal;

    /* ��ʼ���ź������� */
    pstrSem->uiSemOpt = uiSemOpt;

    return RTN_SUCD;
}

/***********************************************************************************
��������: ��ȡһ���ź���, ���ź���Ϊ��ʱ�ɻ�ȡ, ���ź���Ϊ��ʱ�޷���ȡ, ���ܻ�ʹ����
          ��Ϊpend״̬, ���������л�.
��ڲ���: pstrSem: Ҫ��ȡ���ź���ָ��.
          uiDelayTick: ��ȡ�ź����ȴ���ʱ��, ��λ: tick.
                       SEMNOWAIT: ���ȴ�, �����Ƿ��ȡ���ź���������ִ�и�����.
                       SEMWAITFEV: ���õȴ�, ����ȡ�����ź�����������л�Ϊpend״̬
                                   һֱ�ȴ�.
                       ����: ���ڸ�ʱ���ڻ�ȡ���ź���������������pend״̬, ����
                             RTN_SUCD, ���ڸ�ʱ���ڻ�ȡ�����ź���������������pend
                             ״̬, ����RTN_SMTKTO.
�� �� ֵ: RTN_SUCD: ���ӳ�ʱ���ڻ�ȡ���ź���.
          RTN_FAIL: ��ȡ�ź���ʧ��.
          RTN_SMTKTO: �ź���ʱ��ľ�, ��ʱ����.
          RTN_SMTKRT: �����ӳ�״̬���ж�, û�л�ȡ���ź���.
          RTN_SMTKDL: �ź�����ɾ��.
***********************************************************************************/
U32 MDS_SemTake(M_SEM* pstrSem, U32 uiDelayTick)
{
    /* ��ڲ������ */
    if(NULL == pstrSem)
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* �����뵱ǰ������ص��ź��� */
    gpstrCurTcb->pstrSem = pstrSem;

    /* ��ȡ�ź���ʱ���ȴ�ʱ�� */
    if(SEMNOWAIT == uiDelayTick)
    {
        /* �������ź��� */
        if(SEMBIN == (SEMTYPEMASK & pstrSem->uiSemOpt))
        {
            /* �ź���Ϊ��, �ɻ�ȡ���ź��� */
            if(SEMFULL == pstrSem->uiCounter)
            {
                /* ��ȡ���ź������ź�����Ϊ�� */
                pstrSem->uiCounter = SEMEMPTY;

                (void)MDS_IntUnlock();

                return RTN_SUCD;
            }
            else /* �ź���Ϊ��, �޷���ȡ���ź��� */
            {
                (void)MDS_IntUnlock();

                return RTN_SMTKRT;
            }
        }
        else /* �����ź��� */
        {
            /* �ź�����Ϊ��, �ɻ�ȡ���ź��� */
            if(SEMEMPTY != pstrSem->uiCounter)
            {
                /* ��ȡ���ź������ź�������ֵ-1 */
                pstrSem->uiCounter--;

                (void)MDS_IntUnlock();

                return RTN_SUCD;
            }
            else /* �ź���Ϊ��, �޷���ȡ���ź��� */
            {
                (void)MDS_IntUnlock();

                return RTN_SMTKRT;
            }
        }
    }
    else /* ��ȡ�ź���ʱ��Ҫ�ȴ�ʱ�� */
    {
        /* �������ź��� */
        if(SEMBIN == (SEMTYPEMASK & pstrSem->uiSemOpt))
        {
            /* �ź���Ϊ��, �ɻ�ȡ���ź��� */
            if(SEMFULL == pstrSem->uiCounter)
            {
                /* ��ȡ���ź������ź�����Ϊ�� */
                pstrSem->uiCounter = SEMEMPTY;

                (void)MDS_IntUnlock();

                return RTN_SUCD;
            }
            else /* �ź���Ϊ��, �޷���ȡ���ź���, ��Ҫ�л����� */
            {
                /* ��������Ϊpend״̬ */
                if(RTN_FAIL == MDS_TaskPend(pstrSem, uiDelayTick))
                {
                    (void)MDS_IntUnlock();

                    /* ����pendʧ�� */
                    return RTN_FAIL;
                }

                (void)MDS_IntUnlock();

                /* ʹ�����жϵ������� */
                MDS_TaskSwiSched();

                /* ����pend����ֵ, ��ֵ������pend״̬����ʱ��������uiDelayTick�� */
                return gpstrCurTcb->strTaskOpt.uiDelayTick;
            }
        }
        else /* �����ź��� */
        {
            /* �ź�����Ϊ��, �ɻ�ȡ���ź��� */
            if(SEMEMPTY != pstrSem->uiCounter)
            {
                /* ��ȡ���ź������ź�������ֵ-1 */
                pstrSem->uiCounter--;

                (void)MDS_IntUnlock();

                return RTN_SUCD;
            }
            else /* �ź���Ϊ��, �޷���ȡ���ź���, ��Ҫ�л����� */
            {
                /* ��������Ϊpend״̬ */
                if(RTN_FAIL == MDS_TaskPend(pstrSem, uiDelayTick))
                {
                    (void)MDS_IntUnlock();

                    /* ����pendʧ�� */
                    return RTN_FAIL;
                }

                (void)MDS_IntUnlock();

                /* ʹ�����жϵ������� */
                MDS_TaskSwiSched();

                /* ����pend����ֵ, ��ֵ������pend״̬����ʱ��������uiDelayTick�� */
                return gpstrCurTcb->strTaskOpt.uiDelayTick;
            }
        }
    }
}

/***********************************************************************************
��������: �ͷ�һ���ź���, ���ź���Ϊ��ʱ������Ӱ��, ���ź���Ϊ��ʱ, ���ź���������
          ������������Ϊready̬, ���ź���û�������������ź�����Ϊ��.
��ڲ���: pstrSem: Ҫ�ͷŵ��ź���ָ��.
�� �� ֵ: RTN_SUCD: �ͷ��ź����ɹ�.
          RTN_FAIL: �ͷ��ź���ʧ��.
***********************************************************************************/
U32 MDS_SemGive(M_SEM* pstrSem)
{
    M_TCB* pstrTcb;
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U32 uiRtn;
    U8 ucTaskPrio;

    /* ��ڲ������ */
    if(NULL == pstrSem)
    {
        return RTN_FAIL;
    }

    uiRtn = RTN_SUCD;

    (void)MDS_IntLock();

    /* �ź���Ϊ�� */
    if(SEMEMPTY == pstrSem->uiCounter)
    {
        /* �ڱ����ź��������������л�ȡ��Ҫ�ͷŵ����� */
        pstrTcb = MDS_SemGetActiveTask(pstrSem);

        /* ������������, �ͷ����� */
        if(NULL != pstrTcb)
        {
            /* ���ź������ȱ��������� */
            (void)MDS_TaskDelFromSemTab(pstrTcb);

            /* ������delay�����delay���� */
            if(DELAYQUEFLAG == (pstrTcb->uiTaskFlag & DELAYQUEFLAG))
            {
                pstrNode = &pstrTcb->strDelayQue.strQueHead;
                (void)MDS_ChainCurNodeDelete(&gstrDelayTab, pstrNode);

                /* ��������delay���־ */
                pstrTcb->uiTaskFlag &= (~((U32)DELAYQUEFLAG));
            }

            /* ��������pend״̬ */
            pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKPEND);

            /* ����uiDelayTick��������pend����ķ���ֵ */
            pstrTcb->strTaskOpt.uiDelayTick = RTN_SUCD;

            /* ����������ӵ�ready���� */
            pstrNode = &pstrTcb->strTcbQue.strQueHead;
            ucTaskPrio = pstrTcb->ucTaskPrio;
            pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
            pstrPrioFlag = &gstrReadyTab.strFlag;

            MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);

            /* ���������ready״̬ */
            pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;

            (void)MDS_IntUnlock();

            /* ʹ�����жϵ������� */
            MDS_TaskSwiSched();

            return uiRtn;
        }
        else /* û������������ */
        {
            /* �������ź��� */
            if(SEMBIN == (SEMTYPEMASK & pstrSem->uiSemOpt))
            {
                /* �ͷ��ź������ź�����Ϊ�� */
                pstrSem->uiCounter = SEMFULL;
            }
            else /* �����ź��� */
            {
                /* �ͷ��ź������ź���+1. �ߴ˷�֧�ź���+1������� */
                pstrSem->uiCounter++;
            }
        }
    }
    else /* �ź����ǿ� */
    {
        /* �����ź��� */
        if(SEMCNT == (SEMTYPEMASK & pstrSem->uiSemOpt))
        {
            /* �ź���δ�� */
            if(SEMFULL != pstrSem->uiCounter)
            {
                /* �ͷ��ź������ź���+1 */
                pstrSem->uiCounter++;
            }
            else /* �ź������� */
            {
                uiRtn = RTN_SMGVOV;
            }
        }
    }

    (void)MDS_IntUnlock();

    return uiRtn;
}

/***********************************************************************************
��������: �ͷű����ź�������������������, ȥ����Щ�����pend״̬, ��ָ�������������
          ����ֵ.
��ڲ���: pstrSem: Ҫ�ͷŵ��ź���ָ��.
          uiRtnValue: ָ������������ķ���ֵ
�� �� ֵ: RTN_SUCD: �ͷ��ź����ɹ�.
          RTN_FAIL: �ͷ��ź���ʧ��.
***********************************************************************************/
U32 MDS_SemFlushValue(M_SEM* pstrSem, U32 uiRtnValue)
{
    M_TCB* pstrTcb;
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* ��ڲ������ */
    if(NULL == pstrSem)
    {
        return RTN_FAIL;
    }

    (void)MDS_IntLock();

    /* �ڱ����ź��������������л�ȡ��Ҫ�ͷŵ����� */
    while(1)
    {
        pstrTcb = MDS_SemGetActiveTask(pstrSem);

        /* ������������, �ͷ����� */
        if(NULL != pstrTcb)
        {
            /* ���ź������ȱ��������� */
            (void)MDS_TaskDelFromSemTab(pstrTcb);

            /* ������delay�����delay���� */
            if(DELAYQUEFLAG == (pstrTcb->uiTaskFlag & DELAYQUEFLAG))
            {
                pstrNode = &pstrTcb->strDelayQue.strQueHead;
                (void)MDS_ChainCurNodeDelete(&gstrDelayTab, pstrNode);

                /* ��������delay���־ */
                pstrTcb->uiTaskFlag &= (~((U32)DELAYQUEFLAG));
            }

            /* ��������pend״̬ */
            pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKPEND);

            /* ����uiDelayTick��������pend����ķ���ֵ */
            pstrTcb->strTaskOpt.uiDelayTick = uiRtnValue;

            /* ����������ӵ�ready���� */
            pstrNode = &pstrTcb->strTcbQue.strQueHead;
            ucTaskPrio = pstrTcb->ucTaskPrio;
            pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
            pstrPrioFlag = &gstrReadyTab.strFlag;

            MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);

            /* ���������ready״̬ */
            pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;
        }
        else /* û������������, ����ѭ������ */
        {
            break;
        }
    }

    /* ���ź�����Ϊ�� */
    pstrSem->uiCounter = SEMEMPTY;

    (void)MDS_IntUnlock();

    /* ʹ�����жϵ������� */
    MDS_TaskSwiSched();

    return RTN_SUCD;
}

/***********************************************************************************
��������: �ͷű����ź�������������������, ȥ����Щ�����pend״̬.
��ڲ���: pstrSem: Ҫ�ͷŵ��ź���ָ��.
�� �� ֵ: RTN_SUCD: �ͷ��ź����ɹ�.
          RTN_FAIL: �ͷ��ź���ʧ��.
***********************************************************************************/
U32 MDS_SemFlush(M_SEM* pstrSem)
{
    /* �ͷ��ź�������������������, ��MDS_SemFlush�ͷŵ��������񷵻سɹ� */
    return MDS_SemFlushValue(pstrSem, RTN_SUCD);
}

/***********************************************************************************
��������: ɾ��һ���ź���.
��ڲ���: pstrSem: Ҫɾ�����ź���ָ��.
�� �� ֵ: RTN_SUCD: ɾ���ź����ɹ�.
          RTN_FAIL: ɾ���ź���ʧ��.
***********************************************************************************/
U32 MDS_SemDelete(M_SEM* pstrSem)
{
    /* ��ڲ������ */
    if(NULL == pstrSem)
    {
        return RTN_FAIL;
    }

    /* �ͷ��ź�������������������, ��MDS_SemDelete�ͷŵ��������񷵻��ź�����ɾ�� */
    if(RTN_SUCD != MDS_SemFlushValue(pstrSem, RTN_SMTKDL))
    {
        return RTN_FAIL;
    }

    return RTN_SUCD;
}

