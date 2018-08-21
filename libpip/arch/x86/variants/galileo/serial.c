#include <pip/api.h>
#include <pip/serial.h>
#include "galileo-support.h"
#define SERIAL_PORT 0x3f8



void Pip_Debug_Putc(char c)
{
    initGalileoSerial(DEBUG_SERIAL);
#ifdef VIRTUAL_SERIAL_PORT
    galileoSerialPrintc(VIRTUAL_SERIAL_PORT);
#endif
    galileoSerialPrintc(c);
}

void Pip_Debug_PutWchar(uint16_t wchar)
{
    initGalileoSerial(DEBUG_SERIAL);
    galileoSerialPrintc(wchar >> 8);
    galileoSerialPrintc(wchar & 0xff);
}
