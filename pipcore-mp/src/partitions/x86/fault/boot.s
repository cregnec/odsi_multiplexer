#define __ASSEMBLY__
#include <pip/vidt.h>


.section .text.entry
.global _start
.extern main
.extern Pip_VCLI
_start:
    # push %ebx #data2
    # push %eax #data1
    call Pip_VCLI
    call main
    call loop

loop:
    jmp loop
