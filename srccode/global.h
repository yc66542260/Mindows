
#ifndef  GLOBAL_H
#define  GLOBAL_H


#include <ADuC7024.h>
#include "mindows.h"
#include "mds_queue.h"


#define TASKSTACK               600     /* 堆栈大小, 单位: 字节 */


#define TIME1INTMASK            0x8     /* Timer1中断使能位 */
#define UARTINTMASK             0x4000  /* UART中断时能位 */


#define BUFPOOLNUM              20      /* BUFPOOL数量 */
#define MSGBUFLEN               50      /* 每个BUF的长度 */


/* 消息缓冲结构 */
typedef struct msgbuf
{
    M_CHAIN strChain;                   /* 缓冲链表 */
    U8 ucLength;                        /* 消息长度 */
    U8 ucCounter;                       /* 消息收发计数 */
    U8 aucBuf[MSGBUFLEN];               /* 消息缓冲 */
}MSGBUF;

/* 消息缓冲池结构 */
typedef struct bufpool
{
    M_CHAIN strFreeList;                /* 缓冲池空闲链表 */
    MSGBUF astrBufPool[BUFPOOLNUM];     /* 缓冲池 */
}BUFPOOL;


/*********************************** 变量声明 *************************************/
extern U8 gaucTask1Stack[TASKSTACK];
extern U8 gaucTask2Stack[TASKSTACK];
extern U8 gaucTask3Stack[TASKSTACK];

extern M_QUE gstrPrtMsgQue;
extern BUFPOOL gstrBufPool;
extern M_CHAIN* gpstrMsgBufPt;

extern M_SEM gstrSemCnt;

/*********************************** 函数声明 *************************************/
extern void DEV_SoftwareInit(void);
extern void DEV_HardwareInit(void);
extern void DEV_DelayMs(U32 uiMs);
extern void DEV_PutStrToMem(U8* pvStringPt, ...);
extern void DEV_IsrInit(void);
extern void DEV_PrintMsg(void);
extern void DEV_BufferInit(BUFPOOL* pstrBufPool);
extern MSGBUF* DEV_BufferAlloc(BUFPOOL* pstrBufPool);
extern void DEV_BufferFree(BUFPOOL* pstrBufPool, M_CHAIN* pstrQueNode);

extern void TEST_TestTask1(void);
extern void TEST_TestTask2(void);
extern void TEST_TestTask3(void);
extern void TEST_TaskCreatePrint(M_TCB* pstrTcb);
extern void TEST_TaskSwitchPrint(M_TCB* pstrOldTcb, M_TCB* pstrNewTcb);
extern void TEST_TaskDeletePrint(M_TCB* pstrTcb);

extern void INT_FiqFunc(void);


#endif

