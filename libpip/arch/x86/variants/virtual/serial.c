#include <pip/api.h>
#include <pip/serial.h>

void Pip_Debug_Putc(char c)
{
#ifdef VIRTUAL_SERIAL_PORT
    Pip_Notify(0, VIRTUAL_SERIAL_INT, ((VIRTUAL_SERIAL_PORT << 8) | ((uint32_t) c & 0xff)), 0);
#else
    Pip_Notify(0, VIRTUAL_SERIAL_INT, (uint32_t) c & 0xff, 0);
#endif
}
