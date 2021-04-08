
#include "test.h"


U8 gaucTask1Stack[TASKSTACK];       /* ����1�Ķ�ջ */
U8 gaucTask2Stack[TASKSTACK];       /* ����2�Ķ�ջ */
U8 gaucTask3Stack[TASKSTACK];       /* ����3�Ķ�ջ */

M_QUE gstrPrtMsgQue;                /* ��ӡ��Ϣ�Ķ���ָ�� */

M_SEM gstrSemCnt;                   /* �����ź��� */


/***********************************************************************************
��������: ���������л�����1, ���ڴ��ӡ�ַ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TestTask1(void)
{
    while(1)
    {
        DEV_PutStrToMem((U8*)"\r\nTask1 is running! Tick is: %d",
                        MDS_SystemTickGet());

        DEV_DelayMs(1000);

        /* �����ͷż����ź���, ������������ */
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);

        (void)MDS_TaskDelay(500);
    }
}

/***********************************************************************************
��������: ���������л�����2, ���ڴ��ӡ�ַ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TestTask2(void)
{
    while(1)
    {
        /* ��ȡ���ź��������� */
        (void)MDS_SemTake(&gstrSemCnt, SEMWAITFEV);

        DEV_PutStrToMem((U8*)"\r\nTask2 is running! Tick is: %d",
                        MDS_SystemTickGet());

        (void)MDS_TaskDelay(200);
    }
}

/***********************************************************************************
��������: ���������л�����3, ���ڴ��ӡ�ַ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TestTask3(void)
{
    while(1)
    {
        /* ��ȡ���ź��������� */
        (void)MDS_SemTake(&gstrSemCnt, SEMWAITFEV);

        DEV_PutStrToMem((U8*)"\r\nTask3 is running! Tick is: %d",
                        MDS_SystemTickGet());

        (void)MDS_TaskDelay(10);
    }
}

/***********************************************************************************
��������: ���񴴽����̴�ӡ������Ϣ.
��ڲ���: pstrTcb: �´����������TCBָ��.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TaskCreatePrint(M_TCB* pstrTcb)
{
    /* ��������ɹ� */
    if((M_TCB*)NULL != pstrTcb)
    {
        DEV_PutStrToMem((U8*)"\r\nTask %s is created! Tick is: %d",
                        pstrTcb->pucTaskName, MDS_SystemTickGet());
    }
    else /* ��������ʧ�� */
    {
        DEV_PutStrToMem((U8*)"\r\nFail to create task! Tick is: %d",
                        MDS_SystemTickGet());
    }
}

/***********************************************************************************
��������: �����л����̴�ӡ������Ϣ.
��ڲ���: pstrOldTcb: �л�ǰ������TCBָ��.
          pstrNewTcb: �л��������TCBָ��.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TaskSwitchPrint(M_TCB* pstrOldTcb, M_TCB* pstrNewTcb)
{
    DEV_PutStrToMem((U8*)"\r\nTask %s ---> Task %s! Tick is: %d",
                    pstrOldTcb->pucTaskName, pstrNewTcb->pucTaskName,
                    MDS_SystemTickGet());
}

/***********************************************************************************
��������: ����ɾ�����̴�ӡ������Ϣ.
��ڲ���: pstrTcb: ��ɾ���������TCBָ��.
�� �� ֵ: none.
***********************************************************************************/
void TEST_TaskDeletePrint(M_TCB* pstrTcb)
{
    DEV_PutStrToMem((U8*)"\r\nTask %s is deleted! Tick is: %d",
                    pstrTcb->pucTaskName, MDS_SystemTickGet());
}

