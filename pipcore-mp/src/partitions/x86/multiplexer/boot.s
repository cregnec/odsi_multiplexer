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

loop:
    jmp loop


.globl timerRoutineAsm;
.align 4
timerRoutineAsm:
    push %ecx
    push %ebx
    push %eax
    .extern timerRoutine
    call timerRoutine



.globl virtualSerialRoutineAsm;
.align 4
virtualSerialRoutineAsm:
    push %ecx
    push %ebx
    push %eax
    .extern virtualSerialRoutine
    call virtualSerialRoutine
