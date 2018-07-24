#include <pip/api.h>

#define VIRTUAL_SERIAL_PORT 0x90

void Pip_Debug_Putc(char c)
{
    Pip_Notify(0, VIRTUAL_SERIAL_PORT, (uint32_t) c & 0xff, 0);
}
