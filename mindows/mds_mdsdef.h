
#ifndef  MDS_MDSDEF_H
#define  MDS_MDSDEF_H


/* ����������� */
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


/* ����ֵ */
#define RTN_SUCD            0       /* �ɹ� */
#define RTN_FAIL            1       /* ʧ�� */


/* ARM����ģʽ���� */
#define MODE_USR            0x10
#define MODE_FIQ            0x11
#define MODE_IRQ            0x12
#define MODE_SVC            0x13
#define MODE_ABT            0x17
#define MODE_UND            0x1B
#define MODE_SYS            0x1F


/* �������ȼ���Ŀ */
#define PRIORITY256         256
#define PRIORITY128         128
#define PRIORITY64          64
#define PRIORITY32          32
#define PRIORITY16          16
#define PRIORITY8           8


/* ���ȱ�궨�� */
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


/* TCB�б��ݼĴ�����Ľṹ��, ������ʱ��������ǰ���ļĴ��� */
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

typedef struct m_chain              /* ����ṹ */
{
    struct m_chain* pstrHead;       /* ͷָ�� */
    struct m_chain* pstrTail;       /* βָ�� */
}M_CHAIN;

typedef struct m_tcbque             /* TCB���нṹ */
{
    M_CHAIN strQueHead;             /* ���Ӷ��е����� */
    struct m_tcb* pstrTcb;          /* TCBָ�� */
}M_TCBQUE;

typedef struct m_taskopt            /* ������� */
{
    U8 ucTaskSta;                   /* ��������״̬ */
    U32 uiDelayTick;                /* �ӳ�ʱ�� */
}M_TASKOPT;

typedef struct m_prioflag               /* ���ȼ���־�� */
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

typedef struct m_taskschedtab           /* ������ȱ� */
{
    M_CHAIN astrChain[PRIORITYNUM];     /* �������ȼ����ڵ� */
    M_PRIOFLAG strFlag;                 /* ���ȼ���־ */
}M_TASKSCHEDTAB;

/* SEM�ṹ�� */
typedef struct m_sem
{
    M_TASKSCHEDTAB strSemTab;       /* �ź������ȱ� */
    U32 uiCounter;                  /* �ź�������ֵ */
    U32 uiSemOpt;                   /* �ź������� */
}M_SEM;

/* TCB�ṹ�� */
typedef struct m_tcb
{
    STACKREG strStackReg;           /* ���ݼĴ����� */
    M_TCBQUE strTcbQue;             /* TCB�ṹ���� */
    M_TCBQUE strDelayQue;           /* delay����� */
    M_SEM* pstrSem;                 /* ��������ص��ź���ָ�� */
    U8* pucTaskName;                /* ��������ָ�� */
    U32 uiTaskFlag;                 /* �����־ */
    U8 ucTaskPrio;                  /* �������ȼ� */
    M_TASKOPT strTaskOpt;           /* ������� */
    U32 uiStillTick;                /* �ӳٵ���ʱ�� */
}M_TCB;

#endif

