#define __ASSEMBLY__
#include <pip/vidt.h>


.section .text.entry
.global _start
.extern main
.extern Pip_VCLI
_start:
    push %ebx #data2
    push %eax #data1
    call Pip_VCLI
    call main
    call loop



.globl timerRoutineAsm
.align 4
timerRoutineAsm :
    push %ecx #caller
    push %ebx #data2
    push %eax #data1
    .extern timerRoutine
    call timerRoutine
    call loop

loop:
    jmp loop

# .globl spuriousIrqRoutineAsm
# .align 4
# spuriousIrqRoutineAsm :
#     push %ecx
#     push %ebx
#     push %eax
#     .extern spuriousIrqRoutine
#     call spuriousIrqRoutine
