#include "serial.h"
#include <pip/compat.h>
#include <pip/api.h>

void enable_serial(VCPU* vcpu){
        mapPageWrapper((uint32_t)EC_BASE, vcpu->partition_entry, EC_BASE);
        mapPageWrapper((uint32_t)UART_MMIO_BSE, vcpu->partition_entry, UART_MMIO_BSE);
        mapPageWrapper((uint32_t)UART_PCI_BSE, vcpu->partition_entry, UART_PCI_BSE);
}

void disable_serial(VCPU* vcpu){
    Pip_RemoveVAddr(vcpu->partition_entry, EC_BASE);
    Pip_RemoveVAddr(vcpu->partition_entry, UART_MMIO_BSE);
    Pip_RemoveVAddr(vcpu->partition_entry, UART_PCI_BSE);
}

