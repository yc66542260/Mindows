
#ifndef  MDS_CORE_A_INNER_H
#define  MDS_CORE_A_INNER_H


.equ CPSR_IRQENABLE,        (~0x80)     /* CPSR�Ĵ�����IRQʹ������ */
.equ CPSR_IRQDISABLE,       0x80        /* CPSR�Ĵ�����IRQ��ֹ���� */
.equ CPSR_FIQENABLE,        (~0x40)     /* CPSR�Ĵ�����FIQʹ������ */
.equ CPSR_FIQDISABLE,       0x40        /* CPSR�Ĵ�����FIQ��ֹ���� */


/*********************************** �������� *************************************/
.extern gpstrCurTaskSpAddr
.extern gpstrNextTaskSpAddr


/*********************************** �������� *************************************/
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

