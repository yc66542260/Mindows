
#include "mds_userboot.h"


U8 gaucRootTaskStack[ROOTTASKSTACK];    /* ������Ķ�ջ */
U8 gaucIdleTaskStack[IDLETASKSTACK];    /* ��������Ķ�ջ */


/***********************************************************************************
��������: ������, �û���������������ʼִ��, ���ȼ����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_RootTask(void)
{
    /* ��ʼ����� */
    DEV_SoftwareInit();

    /* ��ʼ��Ӳ�� */
    DEV_HardwareInit();

    /* ��������1 */
    (void)MDS_TaskCreate((U8*)"Test1", (VFUNC)TEST_TestTask1, gaucTask1Stack,
                         TASKSTACK, 2, (M_TASKOPT*)NULL);

    /* ��������2 */
    (void)MDS_TaskCreate((U8*)"Test2", (VFUNC)TEST_TestTask2, gaucTask2Stack,
                         TASKSTACK, 1, (M_TASKOPT*)NULL);

    /* ��������3 */
    (void)MDS_TaskCreate((U8*)"Test3", (VFUNC)TEST_TestTask3, gaucTask3Stack,
                         TASKSTACK, 3, (M_TASKOPT*)NULL);
}

/***********************************************************************************
��������: ��������, CPU����ʱִ���������, ���ȼ����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void MDS_IdleTask(void)
{
    while(1)
    {
        DEV_PrintMsg();
    }
}

