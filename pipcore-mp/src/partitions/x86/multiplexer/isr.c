#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <pip/api.h>
#include <pip/vidt.h>

uint32_t isr_stack = NULL;

INTERRUPT_HANDLER(timerRoutineAsm,timerRoutine)
Pip_VCLI();
schedule(caller);
}


void init_isr()
{
    if (isr_stack == NULL){
        isr_stack = Pip_AllocPage() + 0x1000 -4 ;
    }

    //32
    Pip_RegisterInterrupt(33, &timerRoutineAsm, isr_stack );
    // registerInterrupt(40, &spuriousIrqRoutineAsm, isr_stack);
}
