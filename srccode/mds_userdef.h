
#ifndef  MDS_USERDEF_H
#define  MDS_USERDEF_H


/******************************** 用户可修改宏定义 ********************************/
/* 任务钩子功能宏定义, 只有定义该宏才可使用 */
#define MDS_INCLUDETASKHOOK


#define TICK                10          /* 操作系统调度周期, 单位: ms */
#define CORECLOCKPMS        41780000    /* 芯片内核振荡频率, 单位: Hz */


#define ROOTTASKSTACK       600         /* 根任务堆栈大小, 单位: 字节 */
#define IDLETASKSTACK       600         /* 空闲任务堆栈大小, 单位: 字节 */

#define PRIORITYNUM         PRIORITY8   /* 操作系统支持的优先级数目 */


#endif

