
#include "interrupt.h"


/***********************************************************************************
函数功能: FIQ中断处理函数.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void INT_FiqFunc(void)
{
    U32 uiFiqStatus;

    uiFiqStatus = FIQSTA;

    /* 串口中断 */
    if(UARTINTMASK == (UARTINTMASK & uiFiqStatus))
    {
        INT_UartPrint();
    }
}

/***********************************************************************************
函数功能: 串口打印函数, 由中断触发打印.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void INT_UartPrint(void)
{
    MSGBUF* pstrMsgBuf;
    U32 uiUartSta;
    U8 ucChar;

    /* 读串口中断状态寄存器清串口中断 */
    uiUartSta = COMIID0;

    /* 发送中断 */
    if(UARTINTTXMASK == (UARTINTTXMASK & uiUartSta))
    {
        /* 没有需要打印的缓冲 */
        if(NULL == gpstrMsgBufPt)
        {
            return;
        }

        /* 获取要发送的字符 */
        pstrMsgBuf = (MSGBUF*)gpstrMsgBufPt;
        ucChar = pstrMsgBuf->aucBuf[pstrMsgBuf->ucCounter++];

        /* 缓冲消息没发送完毕 */
        if(pstrMsgBuf->ucCounter <= pstrMsgBuf->ucLength)
        {
            /* 向串口发一个字符 */
            COMTX = ucChar;
        }
        else /* 缓冲消息发送完毕, 释放缓冲 */
        {
            DEV_BufferFree(&gstrBufPool, gpstrMsgBufPt);
            gpstrMsgBufPt = (M_CHAIN*)NULL;
        }
    }
}

