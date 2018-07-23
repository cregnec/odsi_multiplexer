#ifndef __SERIAL__
#include "scheduler.h"

#define EC_BASE 0xE0000000
#define UART_MMIO_BSE 0x9000B000
#define UART_PCI_BSE 0xE00A5000

void enable_serial(VCPU* vcpu);
void disable_serial(VCPU* vcpu);

#endif
