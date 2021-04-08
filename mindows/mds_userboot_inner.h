
#ifndef MDS_USERBOOT_INNER_H
#define MDS_USERBOOT_INNER_H


/* 软中断服务号, 0x00000000~0x1FFFFFFF为操作系统保留 */
#define SWI_TASKSCHED       0x10001001  /* 任务调度 */
#define SWI_INTENABLE       0x10002001  /* 中断使能 */
#define SWI_INTDISABLE      0x10002002  /* 中断禁止 */


/*********************************** 函数声明 *************************************/
extern void MDS_RootTask(void);
extern void MDS_IdleTask(void);


#endif

