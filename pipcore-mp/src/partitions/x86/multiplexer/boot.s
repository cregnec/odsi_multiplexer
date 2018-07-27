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

.globl dmaAddressRoutineAsm;
.align 4
dmaAddressRoutineAsm:
    push %ecx
    push %ebx
    push %eax
    .extern dmaAddressRoutine
    call dmaAddressRoutine

.globl virtualSerialRoutineAsm;
.align 4
virtualSerialRoutineAsm:
    push %ecx
    push %ebx
    push %eax
    .extern virtualSerialRoutine
    call virtualSerialRoutine

.globl faultRoutineAsm;
.align 4
faultRoutineAsm:
    push %ecx
    push %ebx
    push %eax
    .extern faultRoutine
    call faultRoutine
