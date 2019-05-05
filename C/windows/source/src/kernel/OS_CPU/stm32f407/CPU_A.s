
    .cpu    cortex-m4

    .syntax unified
    .thumb
    .text

    .global CPU_IntDis
    .type CPU_IntDis, %function
CPU_IntDis:
        CPSID   I
        BX      LR


    .global CPU_IntEn
    .type CPU_IntEn, %function
CPU_IntEn:
        CPSIE   I
        BX      LR


    .global CPU_SR_Save
    .type CPU_SR_Save, %function
CPU_SR_Save:
        MRS     R0, PRIMASK                     
        CPSID   I
        BX      LR


    .global CPU_SR_Restore
    .type CPU_SR_Restore, %function
CPU_SR_Restore:                                  
        MSR     PRIMASK, R0
        BX      LR





