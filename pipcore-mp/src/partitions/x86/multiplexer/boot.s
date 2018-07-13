#define __ASSEMBLY__
#include <pip/vidt.h>


.section .text.entry
.global _start
.extern main
.extern Pip_VCLI
_start:
    push %ebx
    call Pip_VCLI
    call main
    call loop

loop:
    jmp loop


.globl timerRoutineAsm
.extern vcli
.align 4
timerRoutineAsm :
    push %ecx #caller
    push %ebx #data2
    push %eax #data1
    .extern timerRoutine
    call timerRoutine

.globl spuriousIrqRoutineAsm
.extern vcli
.align 4
spuriousIrqRoutineAsm :
    push %ecx
    push %ebx
    push %eax
    .extern spuriousIrqRoutine
    call spuriousIrqRoutine
