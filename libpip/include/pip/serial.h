#ifndef __SERIAL__
#define __SERIAL__

#define VIRTUAL_SERIAL_INT 0x90

enum virtual_serial_port {
    KERNEL = 0xf0,
    MULTIPLEXER,
    SECURE,
    FAULT,
    FREERTOS,
    OWNER,
    NETWORK,
    SP1,
    SP2,
    SP3
};
#endif
