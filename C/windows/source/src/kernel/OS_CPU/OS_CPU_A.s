    
    .global  OSTCBCurPtr
    .global  OSTCBHighRdyPtr

    .global  _estack
                                    
    .global  OS_CPU_PendSVHandler

    .cpu    cortex-m3
    .fpu    softvfp
    .syntax unified
    .thumb
    .text

    .equ NVIC_INT_CTRL,0xE000ED04
    .equ NVIC_SYSPRI14,0xE000ED22
    .equ NVIC_PENDSV_PRI,0xFF
    .equ NVIC_PENDSVSET,0x10000000

    .global OSStartHighRdy
    .type OSStartHighRdy, %function
OSStartHighRdy:
    LDR     R0, =NVIC_SYSPRI14             /*; Set the PendSV exception priority*/
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0];

    MOVS    R0, #0                         /* Set the PSP to 0 for initial context switch call*/
    MSR     PSP, R0

    LDR     R0, =NVIC_INT_CTRL             /*; Trigger the PendSV exception (causes context switch)*/
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    CPSIE   I                              /*; Enable interrupts at processor level*/
	
OSStartHang:
    B       OSStartHang                    /*; Should never get here*/

    .global OS_CPU_PendSVHandler
    .type OS_CPU_PendSVHandler, %function

OS_CPU_PendSVHandler:
    CPSID   I                                                  /* ; Prevent interruption during context switch*/

    MRS     R0, PSP                                            /* ; PSP is process stack pointer*/
    CBZ     R0, OS_CPU_PendSVHandler_nosave                    /* ; Skip register save the first time*/

    SUBS    R0, R0, #0x20                                       /*; Save remaining regs r4-11 on process stack*/
    STM     R0, {R4-R11}

    LDR     R1, =OSTCBCurPtr  /* ; OSTCBCurPtr->OSTCBStkPtr = SP;*/
    LDR     R1, [R1]
    STR     R0, [R1]   /* ; R0 is SP of process being switched out*/
                                                               
OS_CPU_PendSVHandler_nosave:

    LDR     R0, =OSTCBCurPtr
    LDR     R1, =OSTCBHighRdyPtr
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2] /*; R0 is new process SP; SP = OSTCBHighRdyPtr->StkPtr;*/
    LDM     R0, {R4-R11}                                        
    ADDS    R0, R0, #0x20

    MSR     PSP, R0                                           
    ORR     LR, LR, #0x04                                      
    CPSIE   I

    BX      LR                                                 

    /*END*/
