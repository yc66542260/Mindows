
#include "mindows_inner.h"


/***********************************************************************************
函数功能: 初始化任务堆栈函数, 用来创建任务运行前的堆栈状态.
入口参数: pstrTcb: 任务的TCB指针.
          vfFuncPointer: 任务运行的函数.
返 回 值: none.
***********************************************************************************/
void MDS_TaskStackInit(M_TCB* pstrTcb, VFUNC vfFuncPointer)
{
    STACKREG* pstrRegSp;

    pstrRegSp = &pstrTcb->strStackReg;          /* 寄存器组地址 */

    /* 对TCB中的寄存器组初始化 */
    pstrRegSp->uiCpsr = MODE_USR;               /* CPSR */
    pstrRegSp->uiR0 = 0;                        /* R0 */
    pstrRegSp->uiR1 = 0;                        /* R1 */
    pstrRegSp->uiR2 = 0;                        /* R2 */
    pstrRegSp->uiR3 = 0;                        /* R3 */
    pstrRegSp->uiR4 = 0;                        /* R4 */
    pstrRegSp->uiR5 = 0;                        /* R5 */
    pstrRegSp->uiR6 = 0;                        /* R6 */
    pstrRegSp->uiR7 = 0;                        /* R7 */
    pstrRegSp->uiR8 = 0;                        /* R8 */
    pstrRegSp->uiR9 = 0;                        /* R9 */
    pstrRegSp->uiR10 = 0;                       /* R10 */
    pstrRegSp->uiR11 = 0;                       /* R11 */
    pstrRegSp->uiR12 = 0;                       /* R12 */
    pstrRegSp->uiR13 = (U32)pstrTcb;            /* R13 */
    pstrRegSp->uiR14 = (U32)MDS_TaskSelfDelete; /* R14 */

    /* 非root任务首次运行可能会在IRQ中断里跳转执行, 函数入口地址需要多加4字节 */
    if(MDS_RootTask != vfFuncPointer)
    {
        pstrRegSp->uiR15 = (U32)vfFuncPointer + 4;  /* R15 */
    }
    else    /* root任务在USE模式下直接执行, 可以直接跳转到函数地址 */
    {
        pstrRegSp->uiR15 = (U32)vfFuncPointer;      /* R15 */
    }
}

/***********************************************************************************
函数功能: tick中断服务函数.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TickIsr(void)
{
    U32 uiIrqStatus;

    /* 读取产生中断的IRQ中断源 */
    uiIrqStatus = IRQSTA;

    /* tick产生中断 */
    if(TIME1INTMASK == (TIME1INTMASK & uiIrqStatus))
    {
        /* 调度任务 */
        MDS_TaskTick();

        /******************* 用户可以在下面区域添加tick中断代码 *******************/

        /* 清除tick定时器中断标志 */
        T1CLRI = 0;

        /******************* 用户可以在上面区域添加tick中断代码 *******************/
    }

    /* 中断源有非tick中断 */
    if(TIME1INTMASK != uiIrqStatus)
    {
        /* 中断源没有tick中断则需要更新一次任务调度变量 */
        if(TIME1INTMASK != (TIME1INTMASK & uiIrqStatus))
        {
            MDS_TaskSwitch(gpstrCurTcb);
        }

        /****************** 用户可以在下面区域添加自定义中断代码 ******************/



        /****************** 用户可以在上面区域添加自定义中断代码 ******************/
    }
}

/***********************************************************************************
函数功能: 软中断服务函数.
入口参数: uiSwiNo: 软中断号.
返 回 值: none.
***********************************************************************************/
void MDS_SwiIsr(U32 uiSwiNo)
{
    /* 软中断产生的任务调度 */
    if(SWI_TASKSCHED == (SWI_TASKSCHED & uiSwiNo))
    {
        /* 调度任务 */
        MDS_TaskReadySched();
    }
    else /* 中断源是非任务调度中断 */
    {
        /* 非任务调度中断引发的中断里需要更新一次任务调度变量 */
        MDS_TaskSwitch(gpstrCurTcb);

        switch(uiSwiNo)
        {
            case SWI_INTENABLE:     /* 中断使能 */

                MDS_IntEnable();

                break;

            case SWI_INTDISABLE:    /* 中断禁止 */

                MDS_IntDisable();

                break;

            /*************** 用户可以在下面区域添加自定义软中断代码 ***************/

            default:

                break;

            /*************** 用户可以在上面区域添加自定义软中断代码 ***************/
        }
    }
}

/***********************************************************************************
函数功能: 软中断任务调度函数, 非user模式下调用该函数不做任何操作.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TaskSwiSched(void)
{
    /* 如果在中断中运行该函数则直接返回 */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return;
    }

    /* 调用软中断, 在软中断服务程序中调度任务 */
    MDS_TaskOccurSwi(SWI_TASKSCHED);
}

/***********************************************************************************
函数功能: 判断当前程序是否是在中断中运行.
入口参数: none.
返 回 值: RTN_SUCD: 在中断中运行.
          RTN_FAIL: 不在中断中运行.
***********************************************************************************/
U32 MDS_RunInInt(void)
{
    /* 当前程序在中断中运行 */
    if(MODE_USR != MDS_GetChipMode())
    {
        return RTN_SUCD;
    }
    else
    {
        return RTN_FAIL;
    }
}

