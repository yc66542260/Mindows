
#include "mindows_inner.h"


/***********************************************************************************
��������: ��ʼ�������ջ����, ����������������ǰ�Ķ�ջ״̬.
��ڲ���: pstrTcb: �����TCBָ��.
          vfFuncPointer: �������еĺ���.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskStackInit(M_TCB* pstrTcb, VFUNC vfFuncPointer)
{
    STACKREG* pstrRegSp;

    pstrRegSp = &pstrTcb->strStackReg;          /* �Ĵ������ַ */

    /* ��TCB�еļĴ������ʼ�� */
    pstrRegSp->uiCpsr = MODE_USR;               /* CPSR */
    pstrRegSp->uiR0 = 0;                        /* R0 */
    pstrRegSp->uiR1 = 0;                        /* R1 */
    pstrRegSp->uiR2 = 0;                        /* R2 */
    pstrRegSp->uiR3 = 0;                        /* R3 */
    pstrRegSp->uiR4 = 0;                        /* R4 */
    pstrRegSp->uiR5 = 0;                        /* R5 */
    pstrRegSp->uiR6 = 0;                        /* R6 */
    pstrRegSp->uiR7 = 0;                        /* R7 */
    pstrRegSp->uiR8 = 0;                        /* R8 */
    pstrRegSp->uiR9 = 0;                        /* R9 */
    pstrRegSp->uiR10 = 0;                       /* R10 */
    pstrRegSp->uiR11 = 0;                       /* R11 */
    pstrRegSp->uiR12 = 0;                       /* R12 */
    pstrRegSp->uiR13 = (U32)pstrTcb;            /* R13 */
    pstrRegSp->uiR14 = (U32)MDS_TaskSelfDelete; /* R14 */

    /* ��root�����״����п��ܻ���IRQ�ж�����תִ��, ������ڵ�ַ��Ҫ���4�ֽ� */
    if(MDS_RootTask != vfFuncPointer)
    {
        pstrRegSp->uiR15 = (U32)vfFuncPointer + 4;  /* R15 */
    }
    else    /* root������USEģʽ��ֱ��ִ��, ����ֱ����ת��������ַ */
    {
        pstrRegSp->uiR15 = (U32)vfFuncPointer;      /* R15 */
    }
}

/***********************************************************************************
��������: tick�жϷ�����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TickIsr(void)
{
    U32 uiIrqStatus;

    /* ��ȡ�����жϵ�IRQ�ж�Դ */
    uiIrqStatus = IRQSTA;

    /* tick�����ж� */
    if(TIME1INTMASK == (TIME1INTMASK & uiIrqStatus))
    {
        /* �������� */
        MDS_TaskTick();

        /******************* �û������������������tick�жϴ��� *******************/

        /* ���tick��ʱ���жϱ�־ */
        T1CLRI = 0;

        /******************* �û������������������tick�жϴ��� *******************/
    }

    /* �ж�Դ�з�tick�ж� */
    if(TIME1INTMASK != uiIrqStatus)
    {
        /* �ж�Դû��tick�ж�����Ҫ����һ��������ȱ��� */
        if(TIME1INTMASK != (TIME1INTMASK & uiIrqStatus))
        {
            MDS_TaskSwitch(gpstrCurTcb);
        }

        /****************** �û�������������������Զ����жϴ��� ******************/



        /****************** �û�������������������Զ����жϴ��� ******************/
    }
}

/***********************************************************************************
��������: ���жϷ�����.
��ڲ���: uiSwiNo: ���жϺ�.
�� �� ֵ: none.
***********************************************************************************/
void MDS_SwiIsr(U32 uiSwiNo)
{
    /* ���жϲ������������ */
    if(SWI_TASKSCHED == (SWI_TASKSCHED & uiSwiNo))
    {
        /* �������� */
        MDS_TaskReadySched();
    }
    else /* �ж�Դ�Ƿ���������ж� */
    {
        /* ����������ж��������ж�����Ҫ����һ��������ȱ��� */
        MDS_TaskSwitch(gpstrCurTcb);

        switch(uiSwiNo)
        {
            case SWI_INTENABLE:     /* �ж�ʹ�� */

                MDS_IntEnable();

                break;

            case SWI_INTDISABLE:    /* �жϽ�ֹ */

                MDS_IntDisable();

                break;

            /*************** �û�������������������Զ������жϴ��� ***************/

            default:

                break;

            /*************** �û�������������������Զ������жϴ��� ***************/
        }
    }
}

/***********************************************************************************
��������: ���ж�������Ⱥ���, ��userģʽ�µ��øú��������κβ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_TaskSwiSched(void)
{
    /* ������ж������иú�����ֱ�ӷ��� */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return;
    }

    /* �������ж�, �����жϷ�������е������� */
    MDS_TaskOccurSwi(SWI_TASKSCHED);
}

/***********************************************************************************
��������: �жϵ�ǰ�����Ƿ������ж�������.
��ڲ���: none.
�� �� ֵ: RTN_SUCD: ���ж�������.
          RTN_FAIL: �����ж�������.
***********************************************************************************/
U32 MDS_RunInInt(void)
{
    /* ��ǰ�������ж������� */
    if(MODE_USR != MDS_GetChipMode())
    {
        return RTN_SUCD;
    }
    else
    {
        return RTN_FAIL;
    }
}

