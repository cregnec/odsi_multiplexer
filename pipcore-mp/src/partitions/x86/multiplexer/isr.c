#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>
#include <pip/paging.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/compat.h>
#include <pip/fpinfo.h>

INTERRUPT_HANDLER(timerRoutineAsm,timerRoutine)
// printf("timerRoutine\r\n");
// switch (data1) {
// case queueCreate:
//       queueCreateService(data2);
//   break;
// case queueSend:
//   queueSendService(data2);
//   break;
// case queueReceive:
//   queueReceiveService(data2);
//   break;
// case sbrk:
//   sbrkService(data2);
//   break;
// case channelCom:
//     channelService(data2);
//     break;
// default:
//   __asm__ volatile("call vPortTimerHandler");
// }
//   //__asm__ volatile("call vPortTimerHandler");
END_OF_INTERRUPT

INTERRUPT_HANDLER(spuriousIrqRoutineAsm,spuriousIrqRoutine)
// printf("spuriousIrqRoutine, caller is 0x%x\r\n", caller);
END_OF_INTERRUPT

void init_isr()
{
    registerInterrupt(33, &timerRoutineAsm, (uint32_t*)0xf2d0000 );
    registerInterrupt(40, &spuriousIrqRoutineAsm, (uint32_t*) 0xf2d0000);
}
