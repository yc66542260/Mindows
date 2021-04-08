
#ifndef  MDS_CORE_A_INNER_H
#define  MDS_CORE_A_INNER_H


.equ CPSR_IRQENABLE,        (~0x80)     /* CPSR寄存器中IRQ使能掩码 */
.equ CPSR_IRQDISABLE,       0x80        /* CPSR寄存器中IRQ禁止掩码 */
.equ CPSR_FIQENABLE,        (~0x40)     /* CPSR寄存器中FIQ使能掩码 */
.equ CPSR_FIQDISABLE,       0x40        /* CPSR寄存器中FIQ禁止掩码 */


/*********************************** 变量声明 *************************************/
.extern gpstrCurTaskSpAddr
.extern gpstrNextTaskSpAddr


/*********************************** 函数声明 *************************************/
.global MDS_TickIsr
.global MDS_SwiIsr
.global MDS_TickContextSwitch
.global MDS_SwiContextSwitch
.global MDS_SwitchToTask
.global MDS_TaskOccurSwi
.global MDS_GetChipMode
.global MDS_IntEnable
.global MDS_IntDisable


#endif

