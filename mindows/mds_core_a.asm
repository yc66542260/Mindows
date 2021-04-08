
    .include "mds_core_a_inner.h"


    .text
    .arm


    @��������: ʵ�������л�����, ���Ĵ������ݵ���ǰ�����ջ��, �ӽ�Ҫ���������ջ��
    @          ȡ���Ĵ�������ֵ����ת����Ҫ��������ִ��. �ú�����Tick�жϴ���.
    @��ڲ���: none.
    @�� �� ֵ: none.
    .func MDS_TickContextSwitch
MDS_TickContextSwitch:

    @����ӿڼĴ���
    STMDB  R13!, {R0 - R3, R12, R14}

    @����C����TICK�жϴ�����
    LDR    R0, =MDS_TickIsr
    MOV    R14, PC
    BX     R0

    @���浱ǰ����Ķ�ջ��Ϣ
    LDR    R0, =gpstrCurTaskSpAddr
    LDR    R14, [R0]
    MRS    R0, SPSR
    STMIA  R14!, {R0}
    LDMIA  R13!, {R0 - R3, R12}
    STMIA  R14, {R0 - R14}^
    ADD    R14, R14, #0x3C
    LDMIA  R13!, {R0}
    STMIA  R14, {R0}

    @����������, �ָ���Ҫ���������ֳ�
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


    @��������: ʵ�������л�����, ���Ĵ������ݵ���ǰ�����ջ��, �ӽ�Ҫ���������ջ��
    @          ȡ���Ĵ�������ֵ����ת����Ҫ��������ִ��. �ú����ɳ���������жϺ���
    @          ����.
    @��ڲ���: none.
    @�� �� ֵ: none.
    .func MDS_SwiContextSwitch
MDS_SwiContextSwitch:

    @����ӿڼĴ���
    ADD    R14, R14, #4
    STMDB  R13!, {R0 - R3, R12, R14}

    @����C����SWI�жϴ�����
    LDR    R3, =MDS_SwiIsr
    MOV    R14, PC
    BX     R3

    @���浱ǰ����Ķ�ջ��Ϣ
    LDR    R0, =gpstrCurTaskSpAddr
    LDR    R14, [R0]
    MRS    R0, SPSR
    STMIA  R14!, {R0}
    LDMIA  R13!, {R0 - R3, R12}
    STMIA  R14, {R0 - R14}^
    ADD    R14, R14, #0x3C
    LDMIA  R13!, {R0}
    STMIA  R14, {R0}

    @����������, �ָ���Ҫ���������ֳ�
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


    @��������: ʵ�ִ��޲���ϵͳ״̬�л�������ϵͳ״̬, ��Root�����ջ��ȡ���Ĵ�����
    @          ��ֵ����ת��Root����ִ��. �ú����ɳ���ֱ�ӵ���.
    @��ڲ���: none.
    @�� �� ֵ: none.
    .func MDS_SwitchToTask
MDS_SwitchToTask:

    @��ȡ��Ҫ���������ָ��
    LDR    R0, =gpstrNextTaskSpAddr
    LDR    R13, [R0]

    @��ȡ��Ҫ��������Ķ�ջ��Ϣ������������
    LDMIA  R13!, {R0}
    MSR    CPSR, R0
    LDMIA  R13, {R0 - R15}

    .endfunc


    @��������: ��ȡARMоƬ�Ĺ���ģʽ.
    @��ڲ���: none.
    @�� �� ֵ: MODE_USR/MODE_FIQ/MODE_IRQ/MODE_SVC/MODE_ABT/MODE_UND/MODE_SYS.
    .func MDS_GetChipMode
MDS_GetChipMode:

    MRS    R0, CPSR
    AND    R0, R0, #0x1F
    BX     R14

    .endfunc


    @��������: �ɳ���ֱ�ӵ���, ����SWI���ж�.
    @��ڲ���: none.
    @�� �� ֵ: R0: ���жϷ����.
    .func MDS_TaskOccurSwi
MDS_TaskOccurSwi:

    SWI    0
    BX     R14

    .endfunc


    @��������: ��CPSR�Ĵ����е�I��F��־λ����, ʹ��IRQ��FIQ�ж�.
    @��ڲ���: none.
    @�� �� ֵ: none.
    .func MDS_IntEnable
MDS_IntEnable:

    MRS    R0, SPSR
    LDR    R1, =(CPSR_IRQENABLE & CPSR_FIQENABLE)
    AND    R0, R0, R1
    MSR    SPSR, R0
    BX     R14

    .endfunc


    @��������: ��CPSR�Ĵ����е�I��F��־λ��λ, ��ֹIRQ��FIQ�ж�.
    @��ڲ���: none.
    @�� �� ֵ: none.
    .func MDS_IntDisable
MDS_IntDisable:

    MRS    R0, SPSR
    LDR    R1, =(CPSR_IRQDISABLE | CPSR_FIQDISABLE)
    ORR    R0, R0, R1
    MSR    SPSR, R0
    BX     R14

    .endfunc


    .end

