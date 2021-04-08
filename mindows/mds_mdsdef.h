
#ifndef  MDS_MDSDEF_H
#define  MDS_MDSDEF_H


/* 定义变量类型 */
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef char                S8;
typedef short               S16;
typedef int                 S32;
typedef void                (*VFUNC)(void);


#ifndef NULL
 #define NULL               ((void*)0)
#endif


/* 返回值 */
#define RTN_SUCD            0       /* 成功 */
#define RTN_FAIL            1       /* 失败 */


/* ARM工作模式掩码 */
#define MODE_USR            0x10
#define MODE_FIQ            0x11
#define MODE_IRQ            0x12
#define MODE_SVC            0x13
#define MODE_ABT            0x17
#define MODE_UND            0x1B
#define MODE_SYS            0x1F


/* 任务优先级数目 */
#define PRIORITY256         256
#define PRIORITY128         128
#define PRIORITY64          64
#define PRIORITY32          32
#define PRIORITY16          16
#define PRIORITY8           8


/* 调度表宏定义 */
#if PRIORITY256 == PRIORITYNUM
 #define PRIOFLAGGRP1               32
 #define PRIOFLAGGRP2               4
#elif PRIORITY128 == PRIORITYNUM
 #define PRIOFLAGGRP1               16
 #define PRIOFLAGGRP2               2
#elif PRIORITY64 == PRIORITYNUM
 #define PRIOFLAGGRP1               8
 #define PRIOFLAGGRP2               1
#elif PRIORITY32 == PRIORITYNUM
 #define PRIOFLAGGRP1               4
 #define PRIOFLAGGRP2               1
#elif PRIORITY16 == PRIORITYNUM
 #define PRIOFLAGGRP1               2
 #define PRIOFLAGGRP2               1
#elif PRIORITY8 == PRIORITYNUM
 #define PRIOFLAGGRP1               1
 #define PRIOFLAGGRP2               1
#else
 #error The priotity num must be defined between 8 and 256, and must be the power of 2.
#endif


/* TCB中备份寄存器组的结构体, 用来临时保存任务前换的寄存器 */
typedef struct stackreg
{
    U32 uiCpsr;
    U32 uiR0;
    U32 uiR1;
    U32 uiR2;
    U32 uiR3;
    U32 uiR4;
    U32 uiR5;
    U32 uiR6;
    U32 uiR7;
    U32 uiR8;
    U32 uiR9;
    U32 uiR10;
    U32 uiR11;
    U32 uiR12;
    U32 uiR13;
    U32 uiR14;
    U32 uiR15;
}STACKREG;

typedef struct m_chain              /* 链表结构 */
{
    struct m_chain* pstrHead;       /* 头指针 */
    struct m_chain* pstrTail;       /* 尾指针 */
}M_CHAIN;

typedef struct m_tcbque             /* TCB队列结构 */
{
    M_CHAIN strQueHead;             /* 连接队列的链表 */
    struct m_tcb* pstrTcb;          /* TCB指针 */
}M_TCBQUE;

typedef struct m_taskopt            /* 任务参数 */
{
    U8 ucTaskSta;                   /* 任务运行状态 */
    U32 uiDelayTick;                /* 延迟时间 */
}M_TASKOPT;

typedef struct m_prioflag               /* 优先级标志表 */
{
#if PRIORITYNUM >= PRIORITY128
    U8 aucPrioFlagGrp1[PRIOFLAGGRP1];
    U8 aucPrioFlagGrp2[PRIOFLAGGRP2];
    U8 ucPrioFlagGrp3;
#elif PRIORITYNUM >= PRIORITY16
    U8 aucPrioFlagGrp1[PRIOFLAGGRP1];
    U8 ucPrioFlagGrp2;
#else
    U8 ucPrioFlagGrp1;
#endif
}M_PRIOFLAG;

typedef struct m_taskschedtab           /* 任务调度表 */
{
    M_CHAIN astrChain[PRIORITYNUM];     /* 各个优先级根节点 */
    M_PRIOFLAG strFlag;                 /* 优先级标志 */
}M_TASKSCHEDTAB;

/* SEM结构体 */
typedef struct m_sem
{
    M_TASKSCHEDTAB strSemTab;       /* 信号量调度表 */
    U32 uiCounter;                  /* 信号量计数值 */
    U32 uiSemOpt;                   /* 信号量参数 */
}M_SEM;

/* TCB结构体 */
typedef struct m_tcb
{
    STACKREG strStackReg;           /* 备份寄存器组 */
    M_TCBQUE strTcbQue;             /* TCB结构队列 */
    M_TCBQUE strDelayQue;           /* delay表队列 */
    M_SEM* pstrSem;                 /* 与任务相关的信号量指针 */
    U8* pucTaskName;                /* 任务名称指针 */
    U32 uiTaskFlag;                 /* 任务标志 */
    U8 ucTaskPrio;                  /* 任务优先级 */
    M_TASKOPT strTaskOpt;           /* 任务参数 */
    U32 uiStillTick;                /* 延迟到的时间 */
}M_TCB;

#endif

