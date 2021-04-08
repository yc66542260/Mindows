
#include "device.h"


BUFPOOL gstrBufPool;                /* ��Ϣ����� */
M_CHAIN* gpstrMsgBufPt;             /* ��Ϣ����ָ�� */


/***********************************************************************************
��������: ��ʼ�����.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_SoftwareInit(void)
{
    /* ��ʼ����Ϣ����� */
    DEV_BufferInit(&gstrBufPool);

    /* ������ӡ��Ϣ�Ķ��� */
    (void)MDS_QueCreate(&gstrPrtMsgQue);

    /* ���������ź��� */
    (void)MDS_SemCreate(&gstrSemCnt, SEMCNT | SEMFIFO, SEMEMPTY);

#ifdef MDS_INCLUDETASKHOOK

    /* �ҽ������Ӻ��� */
    MDS_TaskCreateHookAdd(TEST_TaskCreatePrint);
    MDS_TaskSwitchHookAdd(TEST_TaskSwitchPrint);
    MDS_TaskDeleteHookAdd(TEST_TaskDeletePrint);

#endif

    gpstrMsgBufPt = (M_CHAIN*)NULL;
}

/***********************************************************************************
��������: ��ʼ��Ӳ��.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_HardwareInit(void)
{
    /* ����GPIO�ܽ�ģʽ */
    DEV_SetGpioMode();

    /* ��ʼ��tick��ʱ�� */
    (void)MDS_TickTimerInit(DEV_TimerInit);

    /* ��ʼ������ */
    DEV_UartInit();

    /* ��ʼ���ж� */
    DEV_IsrInit();
}

/***********************************************************************************
��������: ��ʼ���ж�.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_IsrInit(void)
{
    /* ʹ�ܶ�ʱ��1��IRQ�ж� */
    IRQEN = TIME1INTMASK;

    /* �ҽ�FIQ�жϴ����� */
    FIQ = INT_FiqFunc;

    /* ʹ��UATR��FIQ�ж� */
    FIQEN = UARTINTMASK;
}

/***********************************************************************************
��������: ��ʼ������.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_UartInit(void)
{
    COMDIV2 = 0;
    COMIEN0 = 0x03;     /* �򿪴����շ��ж� */

    /* �������COMDIV0��COMDIV1�Ĵ��� */
    COMCON0 = 0x83;
    COMDIV0 = 0x88;     /* 9600������ */
    COMDIV1 = 0;

    /* �������COMRX��COMTX, 1��ʼλ1ֹͣλ8bits */
    COMCON0 = 3;
}

/***********************************************************************************
��������: �򴮿ڷ���һ���ַ�.
��ڲ���: ucChar: ������ַ�.
�� �� ֵ: none.
***********************************************************************************/
void DEV_PutChar(U8 ucChar)
{
    /* ���ڷ���æ״̬ */
    while(0x0 == (COMSTA0 & 0x40))
    {
        ;
    }

    COMTX = ucChar;
}

/***********************************************************************************
��������: ���ַ�����ӡ���ڴ�, ��β������NULL. �ú�����һ����ڲ�������Ϊ�ַ���ָ��,
          �ַ�����ֻ֧��%d/%x/%s��ʽ, �������֧�ֶ��㼰�ַ���.
��ڲ���: pvStringPt: ��һ��������ָ��, ����Ϊ�ַ���ָ��.
          ...: ��������.
�� �� ֵ: none.
***********************************************************************************/
void DEV_PutStrToMem(U8* pvStringPt, ...)
{
    U8** ppucParaPt;
    MSGBUF* pstrMsgBuf;
    U8* pucStrPt1;
    U8* pucStrPt2;
    U32 uiMsgCntInStr;
    U32 uiMsgCntInBuf;
    U32 uiPower;
    U32 uiPara;
    U8 ucNumSign;
    U8 ucNum;
    U8 i;

    /* ����buf */
    pstrMsgBuf = (MSGBUF*)DEV_BufferAlloc(&gstrBufPool);
    if(NULL == pstrMsgBuf)
    {
        return;
    }

    uiMsgCntInStr = 0;
    uiMsgCntInBuf = 0;

    /* ��ȡ��ڲ����ڶ�ջ�е���ʼ��ַ */
    ppucParaPt = &pvStringPt;

    /* ��һ���������ַ��������ڵ�ַ */
    pucStrPt1 = *ppucParaPt;

    /* ָ��ڶ�����ڲ���, ����һ����Ҫת���Ĳ����ĵ�ַ */
    ppucParaPt++;

    /* ѭ�������һ���ַ������� */
    while(0 != pucStrPt1[uiMsgCntInStr])
    {
        /* ��ӡ����Ϣ����buf�������˳� */
        if(uiMsgCntInBuf >= MSGBUFLEN)
        {
            break;
        }

        /* ��%�Ų���Ҫת���ַ�, ֱ��copy��buf */
        if('%' != pucStrPt1[uiMsgCntInStr])
        {
            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = pucStrPt1[uiMsgCntInStr];
            uiMsgCntInStr++;
            uiMsgCntInBuf++;
        }
        else /* ��Ҫת������ */
        {
            /* ָ��%�ź�����ַ� */
            uiMsgCntInStr++;

            /* ������ת��Ϊ10����ASCII�� */
            if('d' == pucStrPt1[uiMsgCntInStr])
            {
                /* 0xFFFFFFFF��Ӧ10���Ƶ������ */
                uiPower = 1000000000;

                ucNumSign = 0;

                uiPara = (U32)*ppucParaPt;

                /* ������Ϊ0, ��Ҫת�� */
                if(0 != uiPara)
                {
                    for(i = 0; i < 10; i++)
                    {
                        ucNum = (U8)(uiPara / uiPower);
                        if(0 != ucNum)
                        {
                            /* ��һ����Ϊ0�ĸ�λ, ��Ҫ��� */
                            ucNumSign = 1;
                        }

                        if(1 == ucNumSign)
                        {
                            /* ��ӡ����Ϣ����buf�������˳� */
                            if(uiMsgCntInBuf >= MSGBUFLEN)
                            {
                                break;
                            }

                            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(ucNum);
                            uiMsgCntInBuf++;
                        }

                        /* ׼�������һλ */
                        uiPara -= ucNum * uiPower;
                        uiPower /= 10;
                    }

                    /* ָ��d������ַ� */
                    uiMsgCntInStr++;
                }
                else /* ����Ϊ0, ֱ�����0 */
                {
                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(0);
                    uiMsgCntInStr++;
                    uiMsgCntInBuf++;
                }

                /* ָ����һ����Ҫת���Ĳ��� */
                ppucParaPt++;
            }
            /* ������ת��Ϊ16����ASCII�� */
            else if('x' == pucStrPt1[uiMsgCntInStr])
            {
                /* 0xFFFFFFFF��Ӧ16���Ƶ������ */
                uiPower = 0x10000000;

                ucNumSign = 0;

                uiPara = (U32)*ppucParaPt;

                /* ������Ϊ0, ��Ҫת�� */
                if(0 != uiPara)
                {
                    for(i = 0; i < 8; i++)
                    {
                        ucNum = (U8)(uiPara / uiPower);
                        if(0 != ucNum)
                        {
                            /* ��һ����Ϊ0�ĸ�λ, ��Ҫ��� */
                            ucNumSign = 1;
                        }

                        if(1 == ucNumSign)
                        {
                            /* ��ӡ����Ϣ����buf�������˳� */
                            if(uiMsgCntInBuf >= MSGBUFLEN)
                            {
                                break;
                            }

                            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(ucNum);
                            uiMsgCntInBuf++;
                        }

                        /* ׼�������һλ */
                        uiPara -= ucNum * uiPower;
                        uiPower /= 16;
                    }

                    /* ָ��d������ַ� */
                    uiMsgCntInStr++;
                }
                else /* ����Ϊ0, ֱ�����0 */
                {
                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(0);
                    uiMsgCntInStr++;
                    uiMsgCntInBuf++;
                }

                /* ָ����һ����Ҫת���Ĳ��� */
                ppucParaPt++;
            }
            /* �������е��ַ���copy������ */
            else if('s' == pucStrPt1[uiMsgCntInStr])
            {
                /* ��ȡ��Ҫת�����ַ���������ָ�� */
                pucStrPt2 = (U8*)(*ppucParaPt);

                /* copy�ַ���, ��������β��NULL */
                for(i = 0; 0 != pucStrPt2[i]; i++)
                {
                    /* ��ӡ����Ϣ����buf�������˳� */
                    if(uiMsgCntInBuf >= MSGBUFLEN)
                    {
                        break;
                    }

                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = pucStrPt2[i];
                    uiMsgCntInBuf++;
                }

                /* ָ��s������ַ� */
                uiMsgCntInStr++;

                /* ָ����һ����Ҫת���Ĳ��� */
                ppucParaPt++;
            }
            /* %�ź�ǹ涨�ַ�����%��Ϊ��ͨ�ַ� */
            else
            {
                /* ��%��copy�������� */
                pstrMsgBuf->aucBuf[uiMsgCntInBuf] = '%';
                uiMsgCntInBuf++;
            }
        }
    }

    /* ���buf���� */
    pstrMsgBuf->ucLength = (U8)uiMsgCntInBuf;
    pstrMsgBuf->ucCounter = 0;

    /* ��buf������� */
    (void)MDS_QuePut(&gstrPrtMsgQue, &pstrMsgBuf->strChain);
}

/***********************************************************************************
��������: ��ʼ����ʱ��.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_TimerInit(void)
{
    /* ����Timer1, Timer1��Ϊϵͳ��tickʱ�� */
    T1LD = TICK * (CORECLOCKPMS / 1000);    /* ����tickʱ�� */
    T1CON = 0xC0;               /* ʹ�ܶ�ʱ��1, ʱ��Ƶ��41.78M, ����ѭ�����¼���  */
}

/***********************************************************************************
��������: ���ø���GPIO�Ĺ���ģʽ.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_SetGpioMode(void)
{
    GP1CON = 0x00000011;    /* P1.0��P1.1����Ϊ���� */
}

/***********************************************************************************
��������: �Ӷ����л�ȡ��Ϣ����, ��ӡ�����еĵ�һ���ַ�, �����ַ����ɴ������жϴ���.
��ڲ���: none.
�� �� ֵ: none.
***********************************************************************************/
void DEV_PrintMsg(void)
{
    MSGBUF* pstrMsgBuf;

    /* û����Ϣ���ڷ���, �Ӷ��л�ȡ��Ϣ׼������ */
    if(NULL == gpstrMsgBufPt)
    {
        /* �Ӷ����л�ȡ����Ҫ���͵���Ϣ�Ļ���, ������Ϣ */
        if(RTN_SUCD == MDS_QueGet(&gstrPrtMsgQue, &gpstrMsgBufPt))
        {
            pstrMsgBuf = (MSGBUF*)gpstrMsgBufPt;

            pstrMsgBuf->ucCounter = 1;

            DEV_PutChar(pstrMsgBuf->aucBuf[0]);
        }
    }
}

/***********************************************************************************
��������: ������ת����ASCII��.
��ڲ���: ucHex: Ҫת����16������.
�� �� ֵ: ת���ɵ�ASCII��.
***********************************************************************************/
U8 DEV_Num2Asc(U8 ucNum)
{
    if(ucNum < 0xA)                     /* ���0~9 */
    {
        return (ucNum + '0');
    }
    else
    {
        return (ucNum - 0x0A + 'A');    /* ���A~F */
    }
}

/***********************************************************************************
��������: ��ASCII��ת��������.
��ڲ���: ucHex: Ҫת����16������.
�� �� ֵ: ת���ɵ�ASCII��.
***********************************************************************************/
U8 DEV_Asc2Num(U8 ucAsc)
{
    if(ucAsc >= 'A')                    /* ���10~15 */
    {
        return (ucAsc - 'A' + 10);
    }
    else if(ucAsc >= 'a')               /* ���10~15 */
    {
        return (ucAsc - 'a' + 10);
    }
    else
    {
        return (ucAsc - '0');           /* ���0~9 */
    }
}

/***********************************************************************************
��������: ��ʼ����Ϣ�����.
��ڲ���: pstrBufPool: ��Ϣ�����ָ��.
�� �� ֵ: none.
***********************************************************************************/
void DEV_BufferInit(BUFPOOL* pstrBufPool)
{
    U32 i;

    /* ��ʼ������ؿ������� */
    MDS_ChainInit(&pstrBufPool->strFreeList);

    /* �����������������б� */
    for(i = 0; i < BUFPOOLNUM; i++)
    {
        MDS_ChainNodeAdd(&pstrBufPool->strFreeList,
                         &pstrBufPool->astrBufPool[i].strChain);
    }
}

/***********************************************************************************
��������: ������Ϣ����.
��ڲ���: pstrBufPool: ��Ϣ�����ָ��.
�� �� ֵ: �������Ϣ����ָ��.
***********************************************************************************/
MSGBUF* DEV_BufferAlloc(BUFPOOL* pstrBufPool)
{
    MSGBUF* pstrBuf;

    (void)MDS_IntLock();

    /* �ӿ��л������һ�� */
    pstrBuf = (MSGBUF*)MDS_ChainNodeDelete(&pstrBufPool->strFreeList);

    (void)MDS_IntUnlock();

    return pstrBuf;
}

/***********************************************************************************
��������: �ͷ���Ϣ����.
��ڲ���: pstrBufPool: ��Ϣ�����ָ��.
          pstrQueNode: �ͷŵĻ���ڵ�ָ��.
�� �� ֵ: none.
***********************************************************************************/
void DEV_BufferFree(BUFPOOL* pstrBufPool, M_CHAIN* pstrQueNode)
{
    (void)MDS_IntLock();

    /* ���ͷŵĻ���ҵ������б� */
    MDS_ChainNodeAdd(&pstrBufPool->strFreeList, pstrQueNode);

    (void)MDS_IntUnlock();
}

