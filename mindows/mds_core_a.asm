
    .include "mds_core_a_inner.h"


    .text
    .arm


    @函数功能: 实现任务切换功能, 将寄存器备份到当前任务堆栈中, 从将要运行任务堆栈中
    @          取出寄存器保存值并跳转到将要运行任务执行. 该函数由Tick中断触发.
    @入口参数: none.
    @返 回 值: none.
    .func MDS_TickContextSwitch
MDS_TickContextSwitch:

    @保存接口寄存器
    STMDB  R13!, {R0 - R3, R12, R14}

    @调用C语言TICK中断处理函数
    LDR    R0, =MDS_TickIsr
    MOV    R14, PC
    BX     R0

    @保存当前任务的堆栈信息
    LDR    R0, =gpstrCurTaskSpAddr
    LDR    R14, [R0]
    MRS    R0, SPSR
    STMIA  R14!, {R0}
    LDMIA  R13!, {R0 - R3, R12}
    STMIA  R14, {R0 - R14}^
    ADD    R14, R14, #0x3C
    LDMIA  R13!, {R0}
    STMIA  R14, {R0}

    @任务调度完毕, 恢复将要运行任务现场
    LDR    R0, =gpstrNextTaskSpAddr
    LDR    R14, [R0]
    LDMIA  R14, {R0}
    MSR    SPSR, R0
    LDMIB  R14, {R0 - R14}^
    NOP
    ADD    R14, R14, #0x40
    LDMIA  R14, {R14}
    SUBS   PC, R14, #4

    .endfunc


    @函数功能: 实现任务切换功能, 将寄存器备份到当前任务堆栈中, 从将要运行任务堆栈中
    @          取出寄存器保存值并跳转到将要运行任务执行. 该函数由程序调用软中断函数
    @          触发.
    @入口参数: none.
    @返 回 值: none.
    .func MDS_SwiContextSwitch
MDS_SwiContextSwitch:

    @保存接口寄存器
    ADD    R14, R14, #4
    STMDB  R13!, {R0 - R3, R12, R14}

    @调用C语言SWI中断处理函数
    LDR    R3, =MDS_SwiIsr
    MOV    R14, PC
    BX     R3

    @保存当前任务的堆栈信息
    LDR    R0, =gpstrCurTaskSpAddr
    LDR    R14, [R0]
    MRS    R0, SPSR
    STMIA  R14!, {R0}
    LDMIA  R13!, {R0 - R3, R12}
    STMIA  R14, {R0 - R14}^
    ADD    R14, R14, #0x3C
    LDMIA  R13!, {R0}
    STMIA  R14, {R0}

    @任务调度完毕, 恢复将要运行任务现场
    LDR    R0, =gpstrNextTaskSpAddr
    LDR    R14, [R0]
    LDMIA  R14, {R0}
    MSR    SPSR, R0
    LDMIB  R14, {R0 - R14}^
    NOP
    ADD    R14, R14, #0x40
    LDMIA  R14, {R14}
    SUB    R14, R14, #4
    MOVS   PC, R14

    .endfunc


    @函数功能: 实现从无操作系统状态切换到操作系统状态, 从Root任务堆栈中取出寄存器保
    @          存值并跳转到Root任务执行. 该函数由程序直接调用.
    @入口参数: none.
    @返 回 值: none.
    .func MDS_SwitchToTask
MDS_SwitchToTask:

    @获取将要运行任务的指针
    LDR    R0, =gpstrNextTaskSpAddr
    LDR    R13, [R0]

    @获取将要运行任务的堆栈信息并运行新任务
    LDMIA  R13!, {R0}
    MSR    CPSR, R0
    LDMIA  R13, {R0 - R15}

    .endfunc


    @函数功能: 获取ARM芯片的工作模式.
    @入口参数: none.
    @返 回 值: MODE_USR/MODE_FIQ/MODE_IRQ/MODE_SVC/MODE_ABT/MODE_UND/MODE_SYS.
    .func MDS_GetChipMode
MDS_GetChipMode:

    MRS    R0, CPSR
    AND    R0, R0, #0x1F
    BX     R14

    .endfunc


    @函数功能: 由程序直接调用, 触发SWI软中断.
    @入口参数: none.
    @返 回 值: R0: 软中断服务号.
    .func MDS_TaskOccurSwi
MDS_TaskOccurSwi:

    SWI    0
    BX     R14

    .endfunc


    @函数功能: 将CPSR寄存器中的I和F标志位清零, 使能IRQ和FIQ中断.
    @入口参数: none.
    @返 回 值: none.
    .func MDS_IntEnable
MDS_IntEnable:

    MRS    R0, SPSR
    LDR    R1, =(CPSR_IRQENABLE & CPSR_FIQENABLE)
    AND    R0, R0, R1
    MSR    SPSR, R0
    BX     R14

    .endfunc


    @函数功能: 将CPSR寄存器中的I和F标志位置位, 禁止IRQ和FIQ中断.
    @入口参数: none.
    @返 回 值: none.
    .func MDS_IntDisable
MDS_IntDisable:

    MRS    R0, SPSR
    LDR    R1, =(CPSR_IRQDISABLE | CPSR_FIQDISABLE)
    ORR    R0, R0, R1
    MSR    SPSR, R0
    BX     R14

    .endfunc


    .end

