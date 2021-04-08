
#include "test.h"


U8 gaucTask1Stack[TASKSTACK];       /* 任务1的堆栈 */
U8 gaucTask2Stack[TASKSTACK];       /* 任务2的堆栈 */
U8 gaucTask3Stack[TASKSTACK];       /* 任务3的堆栈 */

M_QUE gstrPrtMsgQue;                /* 打印消息的队列指针 */

M_SEM gstrSemCnt;                   /* 计数信号量 */


/***********************************************************************************
函数功能: 测试任务切换函数1, 向内存打印字符串.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void TEST_TestTask1(void)
{
    while(1)
    {
        DEV_PutStrToMem((U8*)"\r\nTask1 is running! Tick is: %d",
                        MDS_SystemTickGet());

        DEV_DelayMs(1000);

        /* 连续释放计数信号量, 激活其它任务 */
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);
        (void)MDS_SemGive(&gstrSemCnt);

        (void)MDS_TaskDelay(500);
    }
}

/***********************************************************************************
函数功能: 测试任务切换函数2, 向内存打印字符串.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void TEST_TestTask2(void)
{
    while(1)
    {
        /* 获取到信号量才运行 */
        (void)MDS_SemTake(&gstrSemCnt, SEMWAITFEV);

        DEV_PutStrToMem((U8*)"\r\nTask2 is running! Tick is: %d",
                        MDS_SystemTickGet());

        (void)MDS_TaskDelay(200);
    }
}

/***********************************************************************************
函数功能: 测试任务切换函数3, 向内存打印字符串.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void TEST_TestTask3(void)
{
    while(1)
    {
        /* 获取到信号量才运行 */
        (void)MDS_SemTake(&gstrSemCnt, SEMWAITFEV);

        DEV_PutStrToMem((U8*)"\r\nTask3 is running! Tick is: %d",
                        MDS_SystemTickGet());

        (void)MDS_TaskDelay(10);
    }
}

/***********************************************************************************
函数功能: 任务创建过程打印任务信息.
入口参数: pstrTcb: 新创建的任务的TCB指针.
返 回 值: none.
***********************************************************************************/
void TEST_TaskCreatePrint(M_TCB* pstrTcb)
{
    /* 创建任务成功 */
    if((M_TCB*)NULL != pstrTcb)
    {
        DEV_PutStrToMem((U8*)"\r\nTask %s is created! Tick is: %d",
                        pstrTcb->pucTaskName, MDS_SystemTickGet());
    }
    else /* 创建任务失败 */
    {
        DEV_PutStrToMem((U8*)"\r\nFail to create task! Tick is: %d",
                        MDS_SystemTickGet());
    }
}

/***********************************************************************************
函数功能: 任务切换过程打印任务信息.
入口参数: pstrOldTcb: 切换前的任务TCB指针.
          pstrNewTcb: 切换后的任务TCB指针.
返 回 值: none.
***********************************************************************************/
void TEST_TaskSwitchPrint(M_TCB* pstrOldTcb, M_TCB* pstrNewTcb)
{
    DEV_PutStrToMem((U8*)"\r\nTask %s ---> Task %s! Tick is: %d",
                    pstrOldTcb->pucTaskName, pstrNewTcb->pucTaskName,
                    MDS_SystemTickGet());
}

/***********************************************************************************
函数功能: 任务删除过程打印任务信息.
入口参数: pstrTcb: 被删除的任务的TCB指针.
返 回 值: none.
***********************************************************************************/
void TEST_TaskDeletePrint(M_TCB* pstrTcb)
{
    DEV_PutStrToMem((U8*)"\r\nTask %s is deleted! Tick is: %d",
                    pstrTcb->pucTaskName, MDS_SystemTickGet());
}

