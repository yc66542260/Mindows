#/***********************************************************************/
#/*  This file is part of the uVision/ARM development tools             */
#/*  Copyright KEIL ELEKTRONIK GmbH 2002-2004                           */
#/***********************************************************************/
#/*                                                                     */
#/*  STARTUP.S:  Startup file ADI ADuC702x device series                */
#/*                                                                     */
#/***********************************************************************/


/*
//*** <<< Use Configuration Wizard in Context Menu >>> ***
*/


# *** Startup Code (executed after Reset) ***


# Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

        .equ    Mode_USR,       0x10
        .equ    Mode_FIQ,       0x11
        .equ    Mode_IRQ,       0x12
        .equ    Mode_SVC,       0x13
        .equ    Mode_ABT,       0x17
        .equ    Mode_UND,       0x1B
        .equ    Mode_SYS,       0x1F

        .equ    I_Bit,          0x80    /* when I bit is set, IRQ is disabled */
        .equ    F_Bit,          0x40    /* when F bit is set, FIQ is disabled */



        .equ    Top_Stack,      0x00012000
        .equ    UND_Stack_Size, 0x00000004
        .equ    SVC_Stack_Size, 0x00000200
        .equ    ABT_Stack_Size, 0x00000004
        .equ    FIQ_Stack_Size, 0x00000200
        .equ    IRQ_Stack_Size, 0x00000200
        .equ    USR_Stack_Size, 0x00000200


# MMR definitions
        .equ    MMR_BASE,       0xFFFF0000     /* MMR Base Address */
        .equ    POWKEY1_OFFSET,     0x0404
        .equ    POWCON_OFFSET,      0x0408
        .equ    POWKEY2_OFFSET,     0x040C



        .equ    PLL_SETUP,      1
        .equ    PLLCFG_Val,     0x00000000



        .equ    GPIO_SETUP,     0
        .equ    GPIOBASE,       0xFFFFF400


        .equ    GP0CON_Val,     0x00000000


        .equ    GP1CON_Val,     0x00000000


        .equ    GP2CON_Val,     0x00000000


        .equ    GP3CON_Val,     0x00000000


        .equ    GP4CON_Val,     0x00000000


        .equ    XM_SETUP,       0
        .equ    XMBASE,         0xFFFFF000


        .equ    XM0CON_Val,     0x00000000
        .equ    XM0PAR_Val,     0x000070FF


        .equ    XM1CON_Val,     0x00000000
        .equ    XM1PAR_Val,     0x000070FF


        .equ    XM2CON_Val,     0x00000000
        .equ    XM2PAR_Val,     0x000070FF


        .equ    XM3CON_Val,     0x00000000
        .equ    XM3PAR_Val,     0x000070FF


        .equ    XMCFG_Val,      0x00000001


        .equ    En_StdIO,       0


# Starupt Code must be linked first at Address at which it expects to run.

        .text
        .arm

        .global _startup
        .func   _startup
_startup:


# Exception Vectors
#  Mapped to Address 0.
#  Absolute addressing mode must be used.
#  Dummy Handlers are implemented as infinite loops which can be modified.

Vectors:        LDR     PC, Reset_Addr
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            /* Reserved Vector */
                LDR     PC, IRQ_Addr
                LDR     PC, FIQ_Addr

Reset_Addr:     .word   Reset_Handler
Undef_Addr:     .word   ADI_UNDEF_Interrupt_Setup
SWI_Addr:       .word   MDS_SwiContextSwitch
PAbt_Addr:      .word   ADI_PABORT_Interrupt_Setup
DAbt_Addr:      .word   ADI_DABORT_Interrupt_Setup
IRQ_Addr:       .word   MDS_TickContextSwitch
FIQ_Addr:       .word   ADI_FIQ_Interrupt_Setup

# Reset Handler

Reset_Handler:


.if PLL_SETUP
                LDR     R0, =MMR_BASE
                MOV     R1, #0x01
                STR     R1, [R0,#POWKEY1_OFFSET]
                MOV     R1, #PLLCFG_Val
                STR     R1, [R0,#POWCON_OFFSET]
                MOV     R1, #0xF4
                STR     R1, [R0,#POWKEY2_OFFSET]
.endif


# Setup Pins
.if GPIO_SETUP

                ADR     R10, GPIO_CFG          /* Pointer to GPIO CFG */
                LDMIA   R10, {R0-R5}           /* Load GPIO Configuration */
                STMIA   R0, {R1-R5}            /* Store GPxCON */
                B       GPIO_END

GPIO_CFG:       .word   GPIOBASE
                .word   GP0CON_Val
                .word   GP1CON_Val
                .word   GP2CON_Val
                .word   GP3CON_Val
                .word   GP4CON_Val
GPIO_END:

.endif  /* GPIO_SETUP */


# Setup External Memory Interface
.if XM_SETUP

                ADR     R10, XM_CFG            /* Pointer to XM CFG */
                LDMIA   R10, {R0-R9}           /* Load XM Configuration */
                STR     R1, [R0],#0x10         /* Store XMCFG */
                STMIA   R0, {R2-R9}            /* Store XMxCON & XMxPAR */
                B       XM_END

XM_CFG:         .word   XMBASE
                .word   XMCFG_Val
                .word   XM0CON_Val
                .word   XM1CON_Val
                .word   XM2CON_Val
                .word   XM3CON_Val
                .word   XM0PAR_Val
                .word   XM1PAR_Val
                .word   XM2PAR_Val
                .word   XM3PAR_Val
XM_END:

.endif  /* XM_SETUP */


# Setup Stack for each mode

                LDR     R0, =Top_Stack

#  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #UND_Stack_Size

#  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #ABT_Stack_Size

#  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #FIQ_Stack_Size

#  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #IRQ_Stack_Size

#  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #SVC_Stack_Size

#  Enter User Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_USR
                MOV     SP, R0

#  Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
                SUB     SL, SP, #USR_Stack_Size


# Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel


# Clear .bss section (Zero init)
                MOV     R0, #0
                LDR     R1, =__bss_start__
                LDR     R2, =__bss_end__
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI


# Enter the C code

.if En_StdIO

# Enter the standard system startup code required for stdlib I/O
                B       _start

.else

                LDR     LR, =__Return_from_main
                LDR     R0, =main
                BX      R0

__Return_from_main:
                B       __Return_from_main

.endif


        .size   _startup, . - _startup
        .endfunc


        .end

