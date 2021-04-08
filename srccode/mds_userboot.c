
#include "mds_userboot.h"


U8 gaucRootTaskStack[ROOTTASKSTACK];    /* 根任务的堆栈 */
U8 gaucIdleTaskStack[IDLETASKSTACK];    /* 空闲任务的堆栈 */


/***********************************************************************************
函数功能: 根任务, 用户代码从这个函数开始执行, 优先级最高.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_RootTask(void)
{
    /* 初始化软件 */
    DEV_SoftwareInit();

    /* 初始化硬件 */
    DEV_HardwareInit();

    /* 创建任务1 */
    (void)MDS_TaskCreate((U8*)"Test1", (VFUNC)TEST_TestTask1, gaucTask1Stack,
                         TASKSTACK, 2, (M_TASKOPT*)NULL);

    /* 创建任务2 */
    (void)MDS_TaskCreate((U8*)"Test2", (VFUNC)TEST_TestTask2, gaucTask2Stack,
                         TASKSTACK, 1, (M_TASKOPT*)NULL);

    /* 创建任务3 */
    (void)MDS_TaskCreate((U8*)"Test3", (VFUNC)TEST_TestTask3, gaucTask3Stack,
                         TASKSTACK, 3, (M_TASKOPT*)NULL);
}

/***********************************************************************************
函数功能: 空闲任务, CPU空闲时执行这个任务, 优先级最低.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_IdleTask(void)
{
    while(1)
    {
        DEV_PrintMsg();
    }
}

