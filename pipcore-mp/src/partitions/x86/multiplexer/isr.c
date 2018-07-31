#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/debug.h>
#include "scheduler.h"

uint32_t* irs_stack = NULL;
extern uint32_t _phy_dma_page, _v_dma_page;

INTERRUPT_HANDLER(timerRoutineAsm,timerRoutine)
Pip_VCLI();
schedule(caller);
}

INTERRUPT_HANDLER(dmaAddressRoutineAsm,dmaAddressRoutine)
Pip_VCLI();
DEBUG(INFO, "Notify %x the dma address: 0x%x, 0x%x using page 0x%x\r\n", caller, _phy_dma_page, _v_dma_page, data1);
uint32_t* vaddr = (uint32_t *) Pip_RemoveVAddr(caller, data1);
vaddr[0] = _phy_dma_page;
vaddr[1] = _v_dma_page;
Pip_AddVAddr(vaddr, caller, data1, 1, 1, 1);
resume(caller, 0);
}

INTERRUPT_HANDLER(virtualSerialRoutineAsm,virtualSerialRoutine)
Pip_VCLI();
char c = (char) data1 & 0xff;
Pip_Debug_Putc(c);
//if there is a suspended sub partition in arg 'data2'
if (data2) {
    Pip_Notify(caller, 0x81, data2, 0);
} else {
    resume(caller, 0);
}
}

INTERRUPT_HANDLER(faultRoutineAsm,faultRoutine)
Pip_VCLI();
uint32_t vaddr = (uint32_t) Pip_RemoveVAddr(caller, data2);
int_ctx_t* is = (int_ctx_t*) (vaddr | (data2 & 0xfff));
Pip_AddVAddr(is, caller, data2, 1, 1, 1);
if (is->int_no == 0xe){
    DEBUG(INFO, "Fault %x from 0x%x at 0x%x\r\n", is->int_no, caller, data1);
} else {
    DEBUG(INFO, "Fault %x from 0x%x\r\n", is->int_no, caller);
}
dumpRegs(is, INFO);
mark_task_unrunnable(caller);
schedule(0);
}

void init_isr()
{
    int i = 0;
    if (irs_stack == NULL){
        irs_stack = (uint32_t*)Pip_AllocPage();
        irs_stack = irs_stack + (0x1000/sizeof(uint32_t) - 1);
        DEBUG(TRACE, "Stack address is 0x%x\r\n", irs_stack);
    }

    for (i=1; i<33; i++){
        Pip_RegisterInterrupt(i, &faultRoutineAsm, irs_stack );
    }
    //32
    Pip_RegisterInterrupt(33, &timerRoutineAsm, irs_stack );
    Pip_RegisterInterrupt(0x82, &dmaAddressRoutineAsm, irs_stack );
    Pip_RegisterInterrupt(0x90, &virtualSerialRoutineAsm, irs_stack );
}
