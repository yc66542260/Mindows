
#ifndef  GLOBAL_H
#define  GLOBAL_H


#include <ADuC7024.h>
#include "mindows.h"
#include "mds_queue.h"


#define TASKSTACK               600     /* ��ջ��С, ��λ: �ֽ� */


#define TIME1INTMASK            0x8     /* Timer1�ж�ʹ��λ */
#define UARTINTMASK             0x4000  /* UART�ж�ʱ��λ */


#define BUFPOOLNUM              20      /* BUFPOOL���� */
#define MSGBUFLEN               50      /* ÿ��BUF�ĳ��� */


/* ��Ϣ����ṹ */
typedef struct msgbuf
{
    M_CHAIN strChain;                   /* �������� */
    U8 ucLength;                        /* ��Ϣ���� */
    U8 ucCounter;                       /* ��Ϣ�շ����� */
    U8 aucBuf[MSGBUFLEN];               /* ��Ϣ���� */
}MSGBUF;

/* ��Ϣ����ؽṹ */
typedef struct bufpool
{
    M_CHAIN strFreeList;                /* ����ؿ������� */
    MSGBUF astrBufPool[BUFPOOLNUM];     /* ����� */
}BUFPOOL;


/*********************************** �������� *************************************/
extern U8 gaucTask1Stack[TASKSTACK];
extern U8 gaucTask2Stack[TASKSTACK];
extern U8 gaucTask3Stack[TASKSTACK];

extern M_QUE gstrPrtMsgQue;
extern BUFPOOL gstrBufPool;
extern M_CHAIN* gpstrMsgBufPt;

extern M_SEM gstrSemCnt;

/*********************************** �������� *************************************/
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

