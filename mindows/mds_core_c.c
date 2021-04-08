
#include "mindows_inner.h"


M_TASKSCHEDTAB gstrReadyTab;            /* 任务就绪表 */
M_CHAIN gstrDelayTab;                   /* 任务延迟表 */

U32 guiTick;                            /* 操作系统的tick变量 */

STACKREG* gpstrCurTaskSpAddr;           /* 当前运行的任务的寄存器组指针, 用于从C传递
                                           到汇编 */
STACKREG* gpstrNextTaskSpAddr;          /* 将要运行的任务的寄存器组指针, 用于从C传递
                                           到汇编 */

M_TCB* gpstrCurTcb;                     /* 当前运行的任务的TCB指针 */
M_TCB* gpstrRootTaskTcb;                /* root任务TCB指针 */
M_TCB* gpstrIdleTaskTcb;                /* idle任务TCB指针 */

U32 guiIntLockCounter;                  /* 锁中断计数值 */

const U8 caucTaskPrioUnmapTab[256] =    /* 优先级反向查找表 */
{
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};


/***********************************************************************************
函数功能: 主函数, C程序从此函数开始运行. 该函数创建系统任务并从非任务状态切换到root
          任务运行.
入口参数: none.
返 回 值: none.
***********************************************************************************/
S32 main(void)
{
    /* 初始化系统变量 */
    MDS_SystemVarInit();

    /* 创建根任务 */
    gpstrRootTaskTcb = MDS_TaskCreate((U8*)"Root", (VFUNC)MDS_RootTask,
                   gaucRootTaskStack, ROOTTASKSTACK, HIGHESTPRIO, (M_TASKOPT*)NULL);

    /* 创建空闲任务 */
    gpstrIdleTaskTcb = MDS_TaskCreate((U8*)"Idle", (VFUNC)MDS_IdleTask,
                    gaucIdleTaskStack, IDLETASKSTACK, LOWESTPRIO, (M_TASKOPT*)NULL);

    /* 开始任务调度, 从根任务开始执行 */
    MDS_TaskStart(gpstrRootTaskTcb);

    return 0;
}

/***********************************************************************************
函数功能: 初始化tick定时器.
入口参数: vfFuncPointer: 初始化tick时钟的函数指针.
返 回 值: RTN_SUCD: 初始化tick时钟成功.
          RTN_FAIL: 初始化tick时钟失败.
***********************************************************************************/
U32 MDS_TickTimerInit(VFUNC vfFuncPointer)
{
    /* 入口参数检查 */
    if(NULL == vfFuncPointer)
    {
        return RTN_FAIL;
    }

    /* 初始化tick系统时钟 */
    vfFuncPointer();

    return RTN_SUCD;
}

/***********************************************************************************
函数功能: 初始化系统变量.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_SystemVarInit(void)
{
    /* 将当前运行任务, root任务和idle任务的TCB初始化为NULL */
    gpstrCurTcb = (M_TCB*)NULL;
    gpstrRootTaskTcb = (M_TCB*)NULL;
    gpstrIdleTaskTcb = (M_TCB*)NULL;

    /* 初始化tick从0开始 */
    guiTick = 0;

    /* 初始化锁中断计数为0 */
    guiIntLockCounter = 0;

#ifdef MDS_INCLUDETASKHOOK

    /* 初始化钩子函数 */
    MDS_TaskHookInit();

#endif

    /* 初始化任务ready表 */
    MDS_TaskSchedTabInit(&gstrReadyTab);

    /* 初始化任务delay表 */
    MDS_ChainInit(&gstrDelayTab);
}

/***********************************************************************************
函数功能: tick中断调度任务函数.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TaskTick(void)
{
    /* 每个tick中断tick计数加1 */
    guiTick++;

    /* 调度任务 */
    MDS_TaskSched();
}

/***********************************************************************************
函数功能: 为发生的任务切换做准备, 更新任务切换过程中汇编函数所使用的变量.
入口参数: pstrTcb: 即将运行的任务的TCB指针.
返 回 值: none.
***********************************************************************************/
void MDS_TaskSwitch(M_TCB* pstrTcb)
{
    /* 保存当前任务的TCB备份寄存器组地址, 在汇编语言可以通过这个变量找到任务的备份寄
       存器组地址 */
    gpstrCurTaskSpAddr = &gpstrCurTcb->strStackReg;

    /* 保存即将运行任务的TCB备份寄存器组地址, 在汇编语言可以通过这个变量找到任务的备
       份寄存器组地址 */
    gpstrNextTaskSpAddr = &pstrTcb->strStackReg;

    /* 保存即将运行任务的TCB指针 */
    gpstrCurTcb = pstrTcb;
}

/***********************************************************************************
函数功能: 开始进入任务调度.
入口参数: pstrTcb: 即将运行的任务的TCB指针.
返 回 值: none.
***********************************************************************************/
void MDS_TaskStart(M_TCB* pstrTcb)
{
    /* 保存即将运行任务堆栈指针的地址, 在汇编语言可以通过这个变量获取任务切换后新任
       务的堆栈地址 */
    gpstrNextTaskSpAddr = &pstrTcb->strStackReg;

    /* 保存即将运行任务的TCB指针 */
    gpstrCurTcb = pstrTcb;

    MDS_SwitchToTask();
}

/***********************************************************************************
函数功能: 初始化任务调度表.
入口参数: pstrSchedTab: 调度表指针.
返 回 值: none.
***********************************************************************************/
void MDS_TaskSchedTabInit(M_TASKSCHEDTAB* pstrSchedTab)
{
    U32 i;

    /* 初始化每个优先级的任务ready链表 */
    for(i = 0; i < PRIORITYNUM; i++)
    {
        MDS_ChainInit(&pstrSchedTab->astrChain[i]);
    }

    /* 初始化ready表优先级标志 */
#if PRIORITYNUM >= PRIORITY128

    for(i = 0; i < PRIOFLAGGRP1; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp1[i] = 0;
    }

    for(i = 0; i < PRIOFLAGGRP2; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp2[i] = 0;
    }

    pstrSchedTab->strFlag.ucPrioFlagGrp3 = 0;

#elif PRIORITYNUM >= PRIORITY16

    for(i = 0; i< PRIOFLAGGRP1; i++)
    {
        pstrSchedTab->strFlag.aucPrioFlagGrp1[i] = 0;
    }

    pstrSchedTab->strFlag.ucPrioFlagGrp2 = 0;

#else

    pstrSchedTab->strFlag.ucPrioFlagGrp1 = 0;

#endif
}

/***********************************************************************************
函数功能: 将任务添加到任务调度表.
入口参数: pstrChain: 调度表指针, 将任务添加到该调度表.
          pstrNode: 要添加的任务节点指针.
          pstrPrioFlag: 该调度表对应的优先级标志表指针.
          ucTaskPrio: 要添加的任务的优先级.
返 回 值: none.
***********************************************************************************/
void MDS_TaskAddToSchedTab(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                           M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
    /* 将该任务节点添加到调度表中 */
    MDS_ChainNodeAdd(pstrChain, pstrNode);

    /* 设置该任务对应调度表的优先级标志表 */
    MDS_TaskPrioFlagSet(pstrPrioFlag, ucTaskPrio);
}

/***********************************************************************************
函数功能: 将任务添加到delay表, 按照任务延迟时间从少到多添加到delay表中.
入口参数: pstrNode: 要添加的任务节点指针.
返 回 值: none.
***********************************************************************************/
void MDS_TaskAddToDelayTab(M_CHAIN* pstrNode)
{
    M_CHAIN* pstrTempNode;
    M_CHAIN* pstrNextNode;
    M_TCBQUE* pstrDelayQue;
    U32 uiStillTick;
    U32 uiTempStillTick;

    /* 获取delay表中的第一个任务节点指针 */
    pstrTempNode = MDS_ChainEmpInq(&gstrDelayTab);

    /* delay表非空 */
    if(NULL != pstrTempNode)
    {
        /* 获取要加入delay表的任务的延迟时间 */
        pstrDelayQue = (M_TCBQUE*)pstrNode;
        uiStillTick = pstrDelayQue->pstrTcb->uiStillTick;

        /* 寻找要加入delay表的任务在delay表中的位置 */
        while(1)
        {
            /* 获取delay表中任务的延迟时间 */
            pstrDelayQue = (M_TCBQUE*)pstrTempNode;
            uiTempStillTick = pstrDelayQue->pstrTcb->uiStillTick;

            /* 在以下各条件中判断要加入delay表的任务应该插入的位置 */
            if(uiStillTick < uiTempStillTick)
            {
                if(uiStillTick > guiTick)
                {
                    MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode, pstrNode);

                    return;
                }
                else if(uiStillTick < guiTick)
                {
                    if(guiTick > uiTempStillTick)
                    {
                        MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode,
                                               pstrNode);

                        return;
                    }
                }
            }
            else if(uiStillTick > uiTempStillTick)
            {
                if(uiStillTick > guiTick)
                {
                    if(guiTick > uiTempStillTick)
                    {
                        MDS_ChainCurNodeInsert(&gstrDelayTab, pstrTempNode,
                                               pstrNode);

                        return;
                    }
                }
            }

            /* delay表中取出的任务延迟时间小于要键入delay表的任务延迟时间, 取出下个
               任务节点继续判断 */
            pstrNextNode = MDS_ChainNextNodeEmpInq(&gstrDelayTab, pstrTempNode);

            /* 搜索到delay链表最后一个节点, 将要加入delay表的任务加入到delay表尾 */
            if(NULL == pstrNextNode)
            {
                MDS_ChainNodeAdd(&gstrDelayTab, pstrNode);

                return;
            }
            else /* 更新delay表下个任务节点继续判断 */
            {
                pstrTempNode = pstrNextNode;
            }
        }
    }
    else /* delay表为空则直接将要加入delay表的任务加入到delay表尾 */
    {
        MDS_ChainNodeAdd(&gstrDelayTab, pstrNode);

        return;
    }
}

/***********************************************************************************
函数功能: 从任务调度表删除任务.
入口参数: pstrChain: 调度表指针, 从该链表删除任务.
          pstrPrioFlag: 调度表对应的优先级标志表指针.
          ucTaskPrio: 要删除任务的优先级.
返 回 值: 被删除的任务的节点指针.
***********************************************************************************/
M_CHAIN* MDS_TaskDelFromSchedTab(M_CHAIN* pstrChain, M_PRIOFLAG* pstrPrioFlag,
                                 U8 ucTaskPrio)
{
    M_CHAIN* pstrDelNode;

    /* 将该任务节点从调度表中删除 */
    pstrDelNode = MDS_ChainNodeDelete(pstrChain);

    /* 如果调度表中该优先级为空则清除优先级标志 */
    if(NULL == MDS_ChainEmpInq(pstrChain))
    {
        MDS_TaskPrioFlagClr(pstrPrioFlag, ucTaskPrio);
    }

    /* 返回被删除任务的节点指针 */
    return pstrDelNode;
}

/***********************************************************************************
函数功能: 将任务添加到sem调度表.
入口参数: pstrTcb: 任务的TCB.
          pstrSem: 操作的信号量指针.
返 回 值: none.
***********************************************************************************/
void MDS_TaskAddToSemTab(M_TCB* pstrTcb, M_SEM* pstrSem)
{
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* 如果信号量是采用优先级调度算法 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* 获取任务的相关参数 */
        ucTaskPrio = pstrTcb->ucTaskPrio;
        pstrChain = &pstrSem->strSemTab.astrChain[ucTaskPrio];
        pstrNode = &pstrTcb->strTcbQue.strQueHead;
        pstrPrioFlag = &pstrSem->strSemTab.strFlag;

        /* 添加到sem调度表 */
        MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag, ucTaskPrio);
    }
    else /* 如果信号量采用先进先出调度算法 */
    {
        /* 获取任务的相关参数, 使用0优先级链表作为先进现出链表 */
        pstrChain = &pstrSem->strSemTab.astrChain[LOWESTPRIO];
        pstrNode = &pstrTcb->strTcbQue.strQueHead;

        /* 添加到sem调度表 */
        MDS_ChainNodeAdd(pstrChain, pstrNode);
    }
}

/***********************************************************************************
函数功能: 将任务从sem调度表删除.
入口参数: pstrTcb: 任务的TCB.
返 回 值: 被删除任务的节点指针.
***********************************************************************************/
M_CHAIN* MDS_TaskDelFromSemTab(M_TCB* pstrTcb)
{
    M_SEM* pstrSem;
    M_CHAIN* pstrChain;
    M_PRIOFLAG* pstrPrioFlag;
    U8 ucTaskPrio;

    /* 获取阻塞任务的信号量 */
    pstrSem = pstrTcb->pstrSem;

    /* 如果信号量是采用优先级调度算法 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* 获取任务的相关参数 */
        ucTaskPrio = pstrTcb->ucTaskPrio;
        pstrChain = &pstrSem->strSemTab.astrChain[ucTaskPrio];
        pstrPrioFlag = &pstrSem->strSemTab.strFlag;

        /* 从sem调度表删除当前任务 */
        return MDS_TaskDelFromSchedTab(pstrChain, pstrPrioFlag, ucTaskPrio);
    }
    else /* 如果信号量采用先进先出调度算法 */
    {
        /* 获取任务的相关参数, 使用0优先级链表作为先进现出链表 */
        pstrChain = &pstrSem->strSemTab.astrChain[LOWESTPRIO];

        /* 从sem调度表删除当前任务 */
        return MDS_ChainNodeDelete(pstrChain);
    }
}

/***********************************************************************************
函数功能: 在被该信号量阻塞的任务中获取一个需要被最先释放的任务.
入口参数: pstrSem: 信号量指针.
返 回 值: 需要释放的任务的TCB指针, 若没有需要释放的任务则返回NULL.
***********************************************************************************/
M_TCB* MDS_SemGetActiveTask(M_SEM* pstrSem)
{
    M_CHAIN* pstrNode;
    M_TCBQUE* pstrTaskQue;
    U8 ucTaskPrio;

    /* 信号量采用优先级调度算法 */
    if(SEMPRIO == (SEMSCHEDOPTMASK & pstrSem->uiSemOpt))
    {
        /* 获取信号量表中优先级最高的任务TCB */
        ucTaskPrio = MDS_TaskHighestPrioGet(&pstrSem->strSemTab.strFlag);
    }
    else /* 信号量采用先进先出调度算法 */
    {
        /* 采用0优先级的链表 */
        ucTaskPrio = LOWESTPRIO;
    }

    pstrNode = MDS_ChainEmpInq(&pstrSem->strSemTab.astrChain[ucTaskPrio]);

    /* 信号量中没有被阻塞的任务 */
    if(NULL == pstrNode)
    {
        return (M_TCB*)NULL;
    }
    else
    {
        pstrTaskQue = (M_TCBQUE*)pstrNode;

        return pstrTaskQue->pstrTcb;
    }
}

/***********************************************************************************
函数功能: 任务调度函数, 任务各种状态转换的调度在此函数实现.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TaskSched(void)
{
    M_TCB* pstrTcb;

    /* 调度delay表任务 */
    MDS_TaskDelayTabSched();

    /* 调度ready表任务 */
    pstrTcb = MDS_TaskReadyTabSched();

#ifdef MDS_INCLUDETASKHOOK

    /* 如果任务切换钩子已经挂接函数则执行该函数 */
    if((VFHSWT)NULL != gvfTaskSwitchHook)
    {
        /* 不同任务之间切换才执行任务切换钩子函数 */
        if(pstrTcb != gpstrCurTcb)
        {
            gvfTaskSwitchHook(gpstrCurTcb, pstrTcb);
        }
    }

#endif

    /* 准备任务切换 */
    MDS_TaskSwitch(pstrTcb);
}

/***********************************************************************************
函数功能: 任务调度函数, ready态任务的调度在此函数实现.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TaskReadySched(void)
{
    M_TCB* pstrTcb;

    /* 调度ready表任务 */
    pstrTcb = MDS_TaskReadyTabSched();

#ifdef MDS_INCLUDETASKHOOK

    /* 如果任务切换钩子已经挂接函数则执行该函数 */
    if((VFHSWT)NULL != gvfTaskSwitchHook)
    {
        /* 不同任务之间切换才执行任务切换钩子函数 */
        if(pstrTcb != gpstrCurTcb)
        {
            gvfTaskSwitchHook(gpstrCurTcb, pstrTcb);
        }
    }

#endif

    /* 准备任务切换 */
    MDS_TaskSwitch(pstrTcb);
}

/***********************************************************************************
函数功能: 对ready表进行操作.
入口参数: none.
返 回 值: none.
***********************************************************************************/
M_TCB* MDS_TaskReadyTabSched(void)
{
    M_TCB* pstrTcb;
    M_TCBQUE* pstrTaskQue;
    U8 ucTaskPrio;

    /* 获取ready表中优先级最高的任务的TCB */
    ucTaskPrio = MDS_TaskHighestPrioGet(&gstrReadyTab.strFlag);
    pstrTaskQue = (M_TCBQUE*)MDS_ChainEmpInq(&gstrReadyTab.astrChain[ucTaskPrio]);
    pstrTcb = pstrTaskQue->pstrTcb;

    return pstrTcb;
}

/***********************************************************************************
函数功能: 对delay表进行操作.
入口参数: none.
返 回 值: none.
***********************************************************************************/
void MDS_TaskDelayTabSched(void)
{
    M_TCB* pstrTcb;
    M_CHAIN* pstrChain;
    M_CHAIN* pstrNode;
    M_CHAIN* pstrDelayNode;
    M_CHAIN* pstrNextNode;
    M_PRIOFLAG* pstrPrioFlag;
    M_TCBQUE* pstrDelayQue;
    U32 uiTick;
    U8 ucTaskPrio;

    /* 获取delay表中的任务节点 */
    pstrDelayNode = MDS_ChainEmpInq(&gstrDelayTab);

    /* delay表中有任务, 调度delay表中的任务 */
    if(NULL != pstrDelayNode)
    {
        /* 判断delay表中任务的延迟时间是否结束 */
        while(1)
        {
            /* 获取delay表中的任务的延迟时间 */
            pstrDelayQue = (M_TCBQUE*)pstrDelayNode;
            pstrTcb = pstrDelayQue->pstrTcb;
            uiTick = pstrTcb->uiStillTick;

            /* 该任务延迟时间到, 从delay表中删除并加入到调度表中 */
            if(uiTick == guiTick)
            {
                /* 从delay表拆除该任务 */
                pstrNextNode = MDS_ChainCurNodeDelete(&gstrDelayTab, pstrDelayNode);

                /* 置任务不在delay表标志 */
                pstrTcb->uiTaskFlag &= (~((U32)DELAYQUEFLAG));

                /* 更新该任务的delay和pend相关参数 */
                /* 如果任务拥有delay状态则从delay状态恢复 */
                if(TASKDELAY == (TASKDELAY & pstrTcb->strTaskOpt.ucTaskSta))
                {
                    /* 清除任务的delay状态 */
                    pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKDELAY);

                    /* 借用uiDelayTick变量保存delay任务的返回值 */
                    pstrTcb->strTaskOpt.uiDelayTick = RTN_TKDLTO;
                }
                /* 如果任务拥有pend状态则从pend状态恢复 */
                else if(TASKPEND == (TASKPEND& pstrTcb->strTaskOpt.ucTaskSta))
                {
                    /* 从信号量调度表拆除任务 */
                    (void)MDS_TaskDelFromSemTab(gpstrCurTcb);

                    /* 清除任务的pend状态 */
                    pstrTcb->strTaskOpt.ucTaskSta &= ~((U8)TASKPEND);

                    /* 借用uiDelayTick变量保存pend任务的返回值 */
                    pstrTcb->strTaskOpt.uiDelayTick = RTN_SMTKTO;
                }

                /* 获取该任务的相关参数 */
                pstrNode = &pstrTcb->strTcbQue.strQueHead;
                ucTaskPrio = pstrTcb->ucTaskPrio;
                pstrChain = &gstrReadyTab.astrChain[ucTaskPrio];
                pstrPrioFlag = &gstrReadyTab.strFlag;

                /* 将该任务添加到ready表中 */
                MDS_TaskAddToSchedTab(pstrChain, pstrNode, pstrPrioFlag,
                                      ucTaskPrio);

                /* 增加任务的ready状态 */
                pstrTcb->strTaskOpt.ucTaskSta |= TASKREADY;

                /* delay表已经调度完毕, 结束对delay表的调度 */
                if(NULL == pstrNextNode)
                {
                    break;
                }
                else /* delay表没调度完, 更新下个节点继续判断 */
                {
                    pstrDelayNode = pstrNextNode;
                }
            }
            else /* 所有任务都没有到时间, 结束对delay表的调度 */
            {
                break;
            }
        }
    }
}

/***********************************************************************************
函数功能: 将该任务添加到任务调度表对应的优先级标志表中.
入口参数: pstrPrioFlag: 调度表对应的优先级标志表指针.
          ucTaskPrio: 任务的优先级.
返 回 值: none.
***********************************************************************************/
void MDS_TaskPrioFlagSet(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
    U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
#endif

    /* 设置调度表对应的优先级标志表 */
#if PRIORITYNUM >= PRIORITY128

    /* 获取优先级标志在第一组和第二组中的组号 */
    ucPrioFlagGrp1 = ucTaskPrio / 8;
    ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;

    /* 获取优先级标志在每一组中的位置 */
    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1 % 8;
    ucPosInGrp3 = ucPrioFlagGrp2;

    /* 在每一组中设置优先级标志 */
    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] |= (U8)(1 << ucPosInGrp1);
    pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] |= (U8)(1 << ucPosInGrp2);
    pstrPrioFlag->ucPrioFlagGrp3 |= (U8)(1 << ucPosInGrp3);

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = ucTaskPrio / 8;

    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1;

    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] |= (U8)(1 << ucPosInGrp1);
    pstrPrioFlag->ucPrioFlagGrp2 |= (U8)(1 << ucPosInGrp2);

#else

    pstrPrioFlag->ucPrioFlagGrp1 |= (U8)(1 << ucTaskPrio);

#endif
}

/***********************************************************************************
函数功能: 将该任务从任务调度表对应的优先级标志表中清除.
入口参数: pstrPrioFlag: 调度表对应的优先级标志表指针.
          ucTaskPrio: 任务的优先级.
返 回 值: none.
***********************************************************************************/
void MDS_TaskPrioFlagClr(M_PRIOFLAG* pstrPrioFlag, U8 ucTaskPrio)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
    U8 ucPosInGrp3;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucPosInGrp1;
    U8 ucPosInGrp2;
#endif

    /* 清除调度表对应的优先级标志表 */
#if PRIORITYNUM >= PRIORITY128

    /* 获取优先级标志在第一组和第二组中的组号 */
    ucPrioFlagGrp1 = ucTaskPrio / 8;
    ucPrioFlagGrp2 = ucPrioFlagGrp1 / 8;

    /* 获取优先级标志在每一组中的位置 */
    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1 % 8;
    ucPosInGrp3 = ucPrioFlagGrp2;

    /* 在每一组中清除优先级标志 */
    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &= ~((U8)(1 << ucPosInGrp1));
    if(0 == pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1])
    {
        pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2] &= ~((U8)(1 << ucPosInGrp2));
        if(0 == pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2])
        {
            pstrPrioFlag->ucPrioFlagGrp3 &= ~((U8)(1 << ucPosInGrp3));
        }
    }

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = ucTaskPrio / 8;

    ucPosInGrp1 = ucTaskPrio % 8;
    ucPosInGrp2 = ucPrioFlagGrp1;

    pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1] &= ~((U8)(1 << ucPosInGrp1));
    if(0 == pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1])
    {
        pstrPrioFlag->ucPrioFlagGrp2 &= ~((U8)(1 << ucPosInGrp2));
    }

#else

    pstrPrioFlag->ucPrioFlagGrp1 &= ~((U8)(1 << ucTaskPrio));

#endif
}

/***********************************************************************************
函数功能: 获取任务调度表中任务的最高优先级.
入口参数: pstrPrioFlag: 调度表的优先级标志表指针.
返 回 值: 任务调度表中的最高优先级.
***********************************************************************************/
U8 MDS_TaskHighestPrioGet(M_PRIOFLAG* pstrPrioFlag)
{
#if PRIORITYNUM >= PRIORITY128
    U8 ucPrioFlagGrp1;
    U8 ucPrioFlagGrp2;
    U8 ucHighestFlagInGrp1;
#elif PRIORITYNUM >= PRIORITY16
    U8 ucPrioFlagGrp1;
    U8 ucHighestFlagInGrp1;
#endif

    /* 获取任务调度表中的最高优先级 */
#if PRIORITYNUM >= PRIORITY128

    ucPrioFlagGrp2 = caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp3];

    ucPrioFlagGrp1 =
                caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp2[ucPrioFlagGrp2]];

    ucHighestFlagInGrp1 = caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp1
                                             [ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1]];

    return (U8)((ucPrioFlagGrp2 * 8 + ucPrioFlagGrp1) * 8 + ucHighestFlagInGrp1);

#elif PRIORITYNUM >= PRIORITY16

    ucPrioFlagGrp1 = caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp2];

    ucHighestFlagInGrp1 =
                caucTaskPrioUnmapTab[pstrPrioFlag->aucPrioFlagGrp1[ucPrioFlagGrp1]];

    return (U8)(ucPrioFlagGrp1 * 8 + ucHighestFlagInGrp1);

#else

    return caucTaskPrioUnmapTab[pstrPrioFlag->ucPrioFlagGrp1];

#endif
}

/***********************************************************************************
函数功能: 获取系统当前的tick.
入口参数: none.
返 回 值: 系统当前的tick.
***********************************************************************************/
U32 MDS_SystemTickGet(void)
{
    return guiTick;
}

/***********************************************************************************
函数功能: 获取系统当前任务的TCB.
入口参数: none.
返 回 值: 系统当前的TCB.
***********************************************************************************/
M_TCB* MDS_CurrentTcbGet(void)
{
    return gpstrCurTcb;
}

/***********************************************************************************
函数功能: 禁止总中断, 非user模式下调用该函数不做任何操作. 必须与MDS_IntUnlock函数成
          对使用.
入口参数: none.
返 回 值: RTN_SUCD: 锁中断成功.
          RTN_FAIL: 锁中断失败.
***********************************************************************************/
U32 MDS_IntLock(void)
{
    /* 非操作系统状态, 不设置中断 */
    if(NULL == gpstrCurTcb)
    {
        return RTN_SUCD;
    }

    /* 如果在中断中运行该函数则直接返回 */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return RTN_SUCD;
    }

    /* 第一次调用该函数才做实际的锁中断操作 */
    if(0 == guiIntLockCounter)
    {
        MDS_TaskOccurSwi(SWI_INTDISABLE);

        guiIntLockCounter++;

        return RTN_SUCD;
    }
    /* 非第一次调用该函数并且小于最大次数则直接返回成功 */
    else if(guiIntLockCounter < 0xFFFFFFFF)
    {
        guiIntLockCounter++;

        return RTN_SUCD;
    }
    else /* 超出最大次数则直接返回失败 */
    {
        return RTN_FAIL;
    }
}

/***********************************************************************************
函数功能: 使能总中断, 非user模式下调用该函数不做任何操作. 必须与MDS_IntLock函数成对
          使用.
入口参数: none.
返 回 值: RTN_SUCD: 解锁中断成功.
          RTN_FAIL: 解锁中断失败.
***********************************************************************************/
U32 MDS_IntUnlock(void)
{
    /* 非操作系统状态, 不设置中断 */
    if(NULL == gpstrCurTcb)
    {
        return RTN_SUCD;
    }

    /* 如果在中断中运行该函数则直接返回 */
    if(RTN_SUCD == MDS_RunInInt())
    {
        return RTN_SUCD;
    }

    /* 非第一次调用该函数直接返回成功 */
    if(guiIntLockCounter > 1)
    {
        guiIntLockCounter--;

        return RTN_SUCD;
    }
    /* 最后一次调用该函数才做实际的解锁中断操作 */
    else if(1 == guiIntLockCounter)
    {
        guiIntLockCounter--;

        MDS_TaskOccurSwi(SWI_INTENABLE);

        return RTN_SUCD;
    }
    else /* 等于0次则直接返回失败 */
    {
        return RTN_FAIL;
    }
}

/***********************************************************************************
函数功能: 初始化链表.
入口参数: pstrChain: 链表根节点指针.
返 回 值: none.
***********************************************************************************/
void MDS_ChainInit(M_CHAIN* pstrChain)
{
    /* 空链表的头尾都指向空节点 */
    pstrChain->pstrHead = (M_CHAIN*)NULL;
    pstrChain->pstrTail = (M_CHAIN*)NULL;
}

/***********************************************************************************
函数功能: 向链表添加一个节点, 从链表尾部加入.
入口参数: pstrChain: 链表根节点指针.
          pstrNode: 加入的节点指针.
返 回 值: none.
***********************************************************************************/
void MDS_ChainNodeAdd(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* 链表非空 */
    if(NULL != pstrChain->pstrTail)
    {
        /* 新节点的头指向原尾节点 */
        pstrNode->pstrHead = pstrChain->pstrHead;

        /* 新节点的尾指向根节点 */
        pstrNode->pstrTail = pstrChain;

        /* 原尾节点的尾指向新节点 */
        pstrChain->pstrHead->pstrTail = pstrNode;

        /* 根节点的头指向新加入的节点 */
        pstrChain->pstrHead = pstrNode;
    }
    else /* 链表为空 */
    {
        /* 新节点的头尾都指向根节点 */
        pstrNode->pstrHead = pstrChain;
        pstrNode->pstrTail = pstrChain;

        /* 根节点的头尾都指向新节点 */
        pstrChain->pstrHead = pstrNode;
        pstrChain->pstrTail = pstrNode;
    }
}

/***********************************************************************************
函数功能: 从链表删除一个节点, 从链表头部删除.
入口参数: pstrChain: 链表根节点指针.
返 回 值: 删除的节点指针, 若链表为空则返回NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainNodeDelete(M_CHAIN* pstrChain)
{
    M_CHAIN* pstrTempNode;

    /* 链表中的第一个节点 */
    pstrTempNode = pstrChain->pstrTail;

    /* 链表非空 */
    if(NULL != pstrTempNode)
    {
        /* 链表中有多个节点 */
        if(pstrChain->pstrHead != pstrChain->pstrTail)
        {
            /* 根节点的尾指向第二个节点 */
            pstrChain->pstrTail = pstrTempNode->pstrTail;

            /* 第二个节点的头指向根节点 */
            pstrTempNode->pstrTail->pstrHead = pstrChain;
        }
        else /* 链表中只有一个节点 */
        {
            /* 取出节点后链表为空 */
            pstrChain->pstrHead = (M_CHAIN*)NULL;
            pstrChain->pstrTail = (M_CHAIN*)NULL;
        }

        /* 返回取出的节点指针 */
        return pstrTempNode;
    }
    else /* 链表为空返回NULL */
    {
        return (M_CHAIN*)NULL;
    }
}

/***********************************************************************************
函数功能: 向链表指定的节点前插入一个节点.
入口参数: pstrChain: 链表根节点指针.
          pstrNode: 基准节点指针, 将新节点插到该节点前面.
          pstrNewNode: 新插入节点的指针.
返 回 值: none.
***********************************************************************************/
void MDS_ChainCurNodeInsert(M_CHAIN* pstrChain, M_CHAIN* pstrNode,
                            M_CHAIN* pstrNewNode)
{
    /* 基准节点不是根节点 */
    if(pstrChain != pstrNode)
    {
        /* 基准节点的上个节点的尾指向新节点 */
        pstrNode->pstrHead->pstrTail = pstrNewNode;

        /* 新节点的头指向基准节点的上个节点 */
        pstrNewNode->pstrHead = pstrNode->pstrHead;

        /* 新节点的尾指向基准节点 */
        pstrNewNode->pstrTail = pstrNode;

        /* 基准节点的头指向新节点 */
        pstrNode->pstrHead = pstrNewNode;
    }
    else /* 基准节点是根节点 */
    {
        MDS_ChainNodeAdd(pstrChain, pstrNewNode);
    }
}

/***********************************************************************************
函数功能: 从链表删除指定的节点, 并返回下个节点的指针.
入口参数: pstrChain: 链表根节点指针.
          pstrNode: 要删除的节点的指针.
返 回 值: 删除节点的下个节点指针, 若没有下个节点则返回NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainCurNodeDelete(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* 要删除的节点不是根节点 */
    if(pstrChain != pstrNode)
    {
        /* 链表中有多个节点 */
        if((pstrNode->pstrHead != pstrChain) || (pstrNode->pstrTail != pstrChain))
        {
            /* 要删除节点的上个节点的尾指向要删除节点的下个节点 */
            pstrNode->pstrHead->pstrTail = pstrNode->pstrTail;

            /* 要删除节点的下个节点的头指向要删除节点的上个节点 */
            pstrNode->pstrTail->pstrHead = pstrNode->pstrHead;

            /* 返回删除节点的下个节点指针 */
            return pstrNode->pstrTail;
        }
        else /* 链表中只有一个节点 */
        {
            (void)MDS_ChainNodeDelete(pstrChain);

            /* 没有下个节点, 返回NULL */
            return (M_CHAIN*)NULL;
        }
    }
    else /* 删除根节点直接返回NULL */
    {
        return (M_CHAIN*)NULL;
    }
}

/***********************************************************************************
函数功能: 查询链表是否为空.
入口参数: pstrChain: 链表根节点指针.
返 回 值: 若非空则返回第一个节点的指针, 若空则返回NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainEmpInq(M_CHAIN* pstrChain)
{
    return pstrChain->pstrTail;
}

/***********************************************************************************
函数功能: 查询链表中指定节点的下一个节点是否为空.
入口参数: pstrChain: 链表根节点指针.
          pstrNode: 基准节点指针, 查询该节点的下一个节点.
返 回 值: 若指定节点的下一个节点非空则返回下一个节点的指针, 若空则返回NULL.
***********************************************************************************/
M_CHAIN* MDS_ChainNextNodeEmpInq(M_CHAIN* pstrChain, M_CHAIN* pstrNode)
{
    /* 基准节点是最后一个节点则返回NULL */
    if(pstrNode->pstrTail == pstrChain)
    {
        return (M_CHAIN*)NULL;
    }
    else
    {
        return pstrNode->pstrTail;
    }
}

