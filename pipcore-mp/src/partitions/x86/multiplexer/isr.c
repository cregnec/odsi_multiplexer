#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <pip/api.h>
#include <pip/vidt.h>

uint32_t* irs_stack = NULL;

INTERRUPT_HANDLER(timerRoutineAsm,timerRoutine)
Pip_VCLI();
schedule(caller);
}


INTERRUPT_HANDLER(virtualSerialRoutineAsm,virtualSerialRoutine)
Pip_VCLI();
char c = (char) data1 & 0xff;
Pip_Debug_Putc(c);
//if there is a suspended sub partition
if (data2) {
    Pip_Notify(caller, 0x81, data2, 0);
} else {
    resume(caller, 0);
}
}

void init_isr()
{
    if (irs_stack == NULL){
        irs_stack = (uint32_t *) Pip_AllocPage() + 0x1000 -4 ;
    }
    //32
    Pip_RegisterInterrupt(33, &timerRoutineAsm, irs_stack );
    Pip_RegisterInterrupt(0x90, &virtualSerialRoutineAsm, irs_stack );
}
