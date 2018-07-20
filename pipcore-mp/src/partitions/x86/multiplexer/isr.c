#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <pip/api.h>
#include <pip/vidt.h>

extern void *partitionEntry;
extern void *securePartitionEntry;

uint32_t isr_stack = NULL, test_page = NULL;

INTERRUPT_HANDLER(timerRoutineAsm,timerRoutine)
Pip_VCLI();
schedule(caller);
// printf("data1=0x%x, data2=0x%x, caller=0x%x\r\n", data1, data2, caller);
if (data1 == 1){
    if(!Pip_AddVAddr(test_page, partitionEntry, caller, 0x1, 0x1, 0x1)) {
        printf("Ask FreeRTOS to resume a sub-partition: %x\r\n", caller);
        Pip_Notify(partitionEntry, 0x81, caller, 0);
    } else{
        printf("unknown sub-partition: 0x%x\r\n", caller);
        Pip_RemoveVAddr(partitionEntry, caller);
    }
}
Pip_Resume(caller, 0);
}

// INTERRUPT_HANDLER(spuriousIrqRoutineAsm,spuriousIrqRoutine)
// Pip_VCLI();
// printf("spuriousIrqRoutine, caller is 0x%x\r\n", caller);
// END_OF_INTERRUPT

void init_isr()
{
    if (isr_stack == NULL){
        isr_stack = Pip_AllocPage() + 0x1000 -4 ;
    }
    if (test_page == NULL){
        test_page = Pip_AllocPage();
    }
    //32
    Pip_RegisterInterrupt(33, &timerRoutineAsm, isr_stack );
    // registerInterrupt(40, &spuriousIrqRoutineAsm, isr_stack);
}
