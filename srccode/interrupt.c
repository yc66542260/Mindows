
#include "interrupt.h"


/***********************************************************************************
��������: FIQ�жϴ�����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void INT_FiqFunc(void)
{
    U32 uiFiqStatus;

    uiFiqStatus = FIQSTA;

    /* �����ж� */
    if(UARTINTMASK == (UARTINTMASK & uiFiqStatus))
    {
        INT_UartPrint();
    }
}

/***********************************************************************************
��������: ���ڴ�ӡ����, ���жϴ�����ӡ.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void INT_UartPrint(void)
{
    MSGBUF* pstrMsgBuf;
    U32 uiUartSta;
    U8 ucChar;

    /* �������ж�״̬�Ĵ����崮���ж� */
    uiUartSta = COMIID0;

    /* �����ж� */
    if(UARTINTTXMASK == (UARTINTTXMASK & uiUartSta))
    {
        /* û����Ҫ��ӡ�Ļ��� */
        if(NULL == gpstrMsgBufPt)
        {
            return;
        }

        /* ��ȡҪ���͵��ַ� */
        pstrMsgBuf = (MSGBUF*)gpstrMsgBufPt;
        ucChar = pstrMsgBuf->aucBuf[pstrMsgBuf->ucCounter++];

        /* ������Ϣû������� */
        if(pstrMsgBuf->ucCounter <= pstrMsgBuf->ucLength)
        {
            /* �򴮿ڷ�һ���ַ� */
            COMTX = ucChar;
        }
        else /* ������Ϣ�������, �ͷŻ��� */
        {
            DEV_BufferFree(&gstrBufPool, gpstrMsgBufPt);
            gpstrMsgBufPt = (M_CHAIN*)NULL;
        }
    }
}

