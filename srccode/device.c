
#include "device.h"


BUFPOOL gstrBufPool;                /* 消息缓冲池 */
M_CHAIN* gpstrMsgBufPt;             /* 消息缓冲指针 */


/***********************************************************************************
函数功能: 初始化软件.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_SoftwareInit(void)
{
    /* 初始化消息缓冲池 */
    DEV_BufferInit(&gstrBufPool);

    /* 创建打印消息的队列 */
    (void)MDS_QueCreate(&gstrPrtMsgQue);

    /* 创建计数信号量 */
    (void)MDS_SemCreate(&gstrSemCnt, SEMCNT | SEMFIFO, SEMEMPTY);

#ifdef MDS_INCLUDETASKHOOK

    /* 挂接任务钩子函数 */
    MDS_TaskCreateHookAdd(TEST_TaskCreatePrint);
    MDS_TaskSwitchHookAdd(TEST_TaskSwitchPrint);
    MDS_TaskDeleteHookAdd(TEST_TaskDeletePrint);

#endif

    gpstrMsgBufPt = (M_CHAIN*)NULL;
}

/***********************************************************************************
函数功能: 初始化硬件.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_HardwareInit(void)
{
    /* 设置GPIO管脚模式 */
    DEV_SetGpioMode();

    /* 初始化tick定时器 */
    (void)MDS_TickTimerInit(DEV_TimerInit);

    /* 初始化串口 */
    DEV_UartInit();

    /* 初始化中断 */
    DEV_IsrInit();
}

/***********************************************************************************
函数功能: 初始化中断.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_IsrInit(void)
{
    /* 使能定时器1的IRQ中断 */
    IRQEN = TIME1INTMASK;

    /* 挂接FIQ中断处理函数 */
    FIQ = INT_FiqFunc;

    /* 使能UATR的FIQ中断 */
    FIQEN = UARTINTMASK;
}

/***********************************************************************************
函数功能: 初始化串口.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_UartInit(void)
{
    COMDIV2 = 0;
    COMIEN0 = 0x03;     /* 打开串口收发中断 */

    /* 允许访问COMDIV0和COMDIV1寄存器 */
    COMCON0 = 0x83;
    COMDIV0 = 0x88;     /* 9600波特率 */
    COMDIV1 = 0;

    /* 允许访问COMRX和COMTX, 1开始位1停止位8bits */
    COMCON0 = 3;
}

/***********************************************************************************
函数功能: 向串口发送一个字符.
入口参数: ucChar: 输出的字符.
返 回 值: none.
***********************************************************************************/
void DEV_PutChar(U8 ucChar)
{
    /* 串口发送忙状态 */
    while(0x0 == (COMSTA0 & 0x40))
    {
        ;
    }

    COMTX = ucChar;
}

/***********************************************************************************
函数功能: 将字符串打印到内存, 结尾不包含NULL. 该函数第一个入口参数必须为字符串指针,
          字符串内只支持%d/%x/%s格式, 其余参数支持定点及字符串.
入口参数: pvStringPt: 第一个参数的指针, 必须为字符串指针.
          ...: 其他参数.
返 回 值: none.
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

    /* 申请buf */
    pstrMsgBuf = (MSGBUF*)DEV_BufferAlloc(&gstrBufPool);
    if(NULL == pstrMsgBuf)
    {
        return;
    }

    uiMsgCntInStr = 0;
    uiMsgCntInBuf = 0;

    /* 获取入口参数在堆栈中的起始地址 */
    ppucParaPt = &pvStringPt;

    /* 第一个参数中字符串的所在地址 */
    pucStrPt1 = *ppucParaPt;

    /* 指向第二个入口参数, 即第一个需要转换的参数的地址 */
    ppucParaPt++;

    /* 循环处理第一个字符串参数 */
    while(0 != pucStrPt1[uiMsgCntInStr])
    {
        /* 打印的消息超过buf长度则退出 */
        if(uiMsgCntInBuf >= MSGBUFLEN)
        {
            break;
        }

        /* 非%号不需要转换字符, 直接copy到buf */
        if('%' != pucStrPt1[uiMsgCntInStr])
        {
            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = pucStrPt1[uiMsgCntInStr];
            uiMsgCntInStr++;
            uiMsgCntInBuf++;
        }
        else /* 需要转换参数 */
        {
            /* 指向%号后面的字符 */
            uiMsgCntInStr++;

            /* 将参数转换为10进制ASCII码 */
            if('d' == pucStrPt1[uiMsgCntInStr])
            {
                /* 0xFFFFFFFF对应10进制的最大幂 */
                uiPower = 1000000000;

                ucNumSign = 0;

                uiPara = (U32)*ppucParaPt;

                /* 参数不为0, 需要转换 */
                if(0 != uiPara)
                {
                    for(i = 0; i < 10; i++)
                    {
                        ucNum = (U8)(uiPara / uiPower);
                        if(0 != ucNum)
                        {
                            /* 第一个不为0的高位, 需要输出 */
                            ucNumSign = 1;
                        }

                        if(1 == ucNumSign)
                        {
                            /* 打印的消息超过buf长度则退出 */
                            if(uiMsgCntInBuf >= MSGBUFLEN)
                            {
                                break;
                            }

                            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(ucNum);
                            uiMsgCntInBuf++;
                        }

                        /* 准备输出下一位 */
                        uiPara -= ucNum * uiPower;
                        uiPower /= 10;
                    }

                    /* 指向d后面的字符 */
                    uiMsgCntInStr++;
                }
                else /* 参数为0, 直接输出0 */
                {
                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(0);
                    uiMsgCntInStr++;
                    uiMsgCntInBuf++;
                }

                /* 指向下一个需要转换的参数 */
                ppucParaPt++;
            }
            /* 将参数转换为16进制ASCII码 */
            else if('x' == pucStrPt1[uiMsgCntInStr])
            {
                /* 0xFFFFFFFF对应16进制的最大幂 */
                uiPower = 0x10000000;

                ucNumSign = 0;

                uiPara = (U32)*ppucParaPt;

                /* 参数不为0, 需要转换 */
                if(0 != uiPara)
                {
                    for(i = 0; i < 8; i++)
                    {
                        ucNum = (U8)(uiPara / uiPower);
                        if(0 != ucNum)
                        {
                            /* 第一个不为0的高位, 需要输出 */
                            ucNumSign = 1;
                        }

                        if(1 == ucNumSign)
                        {
                            /* 打印的消息超过buf长度则退出 */
                            if(uiMsgCntInBuf >= MSGBUFLEN)
                            {
                                break;
                            }

                            pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(ucNum);
                            uiMsgCntInBuf++;
                        }

                        /* 准备输出下一位 */
                        uiPara -= ucNum * uiPower;
                        uiPower /= 16;
                    }

                    /* 指向d后面的字符 */
                    uiMsgCntInStr++;
                }
                else /* 参数为0, 直接输出0 */
                {
                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = DEV_Num2Asc(0);
                    uiMsgCntInStr++;
                    uiMsgCntInBuf++;
                }

                /* 指向下一个需要转换的参数 */
                ppucParaPt++;
            }
            /* 将参数中的字符串copy到缓冲 */
            else if('s' == pucStrPt1[uiMsgCntInStr])
            {
                /* 获取需要转换的字符串参数的指针 */
                pucStrPt2 = (U8*)(*ppucParaPt);

                /* copy字符串, 不包含结尾的NULL */
                for(i = 0; 0 != pucStrPt2[i]; i++)
                {
                    /* 打印的消息超过buf长度则退出 */
                    if(uiMsgCntInBuf >= MSGBUFLEN)
                    {
                        break;
                    }

                    pstrMsgBuf->aucBuf[uiMsgCntInBuf] = pucStrPt2[i];
                    uiMsgCntInBuf++;
                }

                /* 指向s后面的字符 */
                uiMsgCntInStr++;

                /* 指向下一个需要转换的参数 */
                ppucParaPt++;
            }
            /* %号后非规定字符则视%号为普通字符 */
            else
            {
                /* 将%号copy进缓冲区 */
                pstrMsgBuf->aucBuf[uiMsgCntInBuf] = '%';
                uiMsgCntInBuf++;
            }
        }
    }

    /* 填充buf参数 */
    pstrMsgBuf->ucLength = (U8)uiMsgCntInBuf;
    pstrMsgBuf->ucCounter = 0;

    /* 将buf挂入队列 */
    (void)MDS_QuePut(&gstrPrtMsgQue, &pstrMsgBuf->strChain);
}

/***********************************************************************************
函数功能: 初始化定时器.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_TimerInit(void)
{
    /* 设置Timer1, Timer1作为系统的tick时钟 */
    T1LD = TICK * (CORECLOCKPMS / 1000);    /* 设置tick时间 */
    T1CON = 0xC0;               /* 使能定时器1, 时钟频率41.78M, 周期循环向下计数  */
}

/***********************************************************************************
函数功能: 设置各个GPIO的工作模式.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_SetGpioMode(void)
{
    GP1CON = 0x00000011;    /* P1.0和P1.1设置为串口 */
}

/***********************************************************************************
函数功能: 从队列中获取消息缓冲, 打印缓冲中的第一个字符, 其它字符将由触发的中断处理.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void DEV_PrintMsg(void)
{
    MSGBUF* pstrMsgBuf;

    /* 没有消息正在发送, 从队列获取消息准备发送 */
    if(NULL == gpstrMsgBufPt)
    {
        /* 从队列中获取到需要发送的消息的缓冲, 发送消息 */
        if(RTN_SUCD == MDS_QueGet(&gstrPrtMsgQue, &gpstrMsgBufPt))
        {
            pstrMsgBuf = (MSGBUF*)gpstrMsgBufPt;

            pstrMsgBuf->ucCounter = 1;

            DEV_PutChar(pstrMsgBuf->aucBuf[0]);
        }
    }
}

/***********************************************************************************
函数功能: 把数字转换成ASCII码.
入口参数: ucHex: 要转换的16进制数.
返 回 值: 转换成的ASCII码.
***********************************************************************************/
U8 DEV_Num2Asc(U8 ucNum)
{
    if(ucNum < 0xA)                     /* 输出0~9 */
    {
        return (ucNum + '0');
    }
    else
    {
        return (ucNum - 0x0A + 'A');    /* 输出A~F */
    }
}

/***********************************************************************************
函数功能: 把ASCII码转换成数字.
入口参数: ucHex: 要转换的16进制数.
返 回 值: 转换成的ASCII码.
***********************************************************************************/
U8 DEV_Asc2Num(U8 ucAsc)
{
    if(ucAsc >= 'A')                    /* 输出10~15 */
    {
        return (ucAsc - 'A' + 10);
    }
    else if(ucAsc >= 'a')               /* 输出10~15 */
    {
        return (ucAsc - 'a' + 10);
    }
    else
    {
        return (ucAsc - '0');           /* 输出0~9 */
    }
}

/***********************************************************************************
函数功能: 初始化消息缓冲池.
入口参数: pstrBufPool: 消息缓冲池指针.
返 回 值: none.
***********************************************************************************/
void DEV_BufferInit(BUFPOOL* pstrBufPool)
{
    U32 i;

    /* 初始化缓冲池空闲链表 */
    MDS_ChainInit(&pstrBufPool->strFreeList);

    /* 将各个缓冲挂入空闲列表 */
    for(i = 0; i < BUFPOOLNUM; i++)
    {
        MDS_ChainNodeAdd(&pstrBufPool->strFreeList,
                         &pstrBufPool->astrBufPool[i].strChain);
    }
}

/***********************************************************************************
函数功能: 申请消息缓冲.
入口参数: pstrBufPool: 消息缓冲池指针.
返 回 值: 分配的消息缓冲指针.
***********************************************************************************/
MSGBUF* DEV_BufferAlloc(BUFPOOL* pstrBufPool)
{
    MSGBUF* pstrBuf;

    (void)MDS_IntLock();

    /* 从空闲缓冲分配一个 */
    pstrBuf = (MSGBUF*)MDS_ChainNodeDelete(&pstrBufPool->strFreeList);

    (void)MDS_IntUnlock();

    return pstrBuf;
}

/***********************************************************************************
函数功能: 释放消息缓冲.
入口参数: pstrBufPool: 消息缓冲池指针.
          pstrQueNode: 释放的缓冲节点指针.
返 回 值: none.
***********************************************************************************/
void DEV_BufferFree(BUFPOOL* pstrBufPool, M_CHAIN* pstrQueNode)
{
    (void)MDS_IntLock();

    /* 将释放的缓冲挂到空闲列表 */
    MDS_ChainNodeAdd(&pstrBufPool->strFreeList, pstrQueNode);

    (void)MDS_IntUnlock();
}

