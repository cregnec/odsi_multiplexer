#ifndef DEF_DEBUG_H
#define DEF_DEBUG_H

#include <stdint.h>
/**
 * \brief Strings for debugging output.
 */

#define  Critical  0 //!< Critical output
#define  Error     1 //!< Error output
#define  Warning   2 //!< Warning output
#define  Info      3 //!< Information output
#define  Log       4 //!< Log output
#define  Trace     5 //!< Annoying, verbose output


#ifndef LOGLEVEL
#define LOGLEVEL Error
#endif

#define DEBUG(l,a,...) DEBUG_ ## l(a, ##__VA_ARGS__)

#if Critical <= LOGLEVEL
#define DEBUG_CRITICAL(a, ...) printf("CRITICAL " a, ##__VA_ARGS__)
#else
#define DEBUG_CRITICAL(a, ...)
#endif

#if Error <= LOGLEVEL
#define DEBUG_ERROR(a, ...) printf("ERROR " a, ##__VA_ARGS__)
#else
#define DEBUG_ERROR(a, ...)
#endif

#if Warning <= LOGLEVEL
#define DEBUG_WARNING(a, ...) printf("WARNING " a, ##__VA_ARGS__)
#else
#define DEBUG_WARNING(a, ...)
#endif

#if Info <= LOGLEVEL
#define DEBUG_INFO(a, ...) printf("INFO " a, ##__VA_ARGS__)
#else
#define DEBUG_INFO(a, ...)
#endif

#if Log <= LOGLEVEL
#define DEBUG_LOG(a, ...) printf("LOG " a, ##__VA_ARGS__)
#else
#define DEBUG_LOG(a, ...)
#endif

#if Trace <= LOGLEVEL
#define DEBUG_TRACE(a, ...) printf("TRACE [%s:%d] " a, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_TRACE(a, ...)
#endif

#define LOGGER(l,a,...) DEBUG(l,a, ##__VA_ARGS__)


#define SIZEOF_CTX              sizeof(int_ctx_t)
#define GENERAL_REG(a, c)       (((int_ctx_t *)a)->regs.c)
#define OPTIONAL_REG(a, c)      (((int_ctx_t *)a)->c)

#define dumpRegs(is, outputLevel) \
do { \
    DEBUG(outputLevel, "Register dump: eax=%x, ebx=%x, ecx=%x, edx=%x\r\n", \
          GENERAL_REG(is, eax), \
          GENERAL_REG(is, ebx), \
          GENERAL_REG(is, ecx), \
          GENERAL_REG(is, edx)); \
    DEBUG(outputLevel, "               edi=%x, esi=%x, ebp=%x, esp=%x\r\n", \
          GENERAL_REG(is, edi), \
          GENERAL_REG(is, esi), \
          GENERAL_REG(is, ebp), \
          OPTIONAL_REG(is, useresp)); \
    DEBUG(outputLevel, "               cs=%x, ss=%x, eip=%x, int=%x\r\n", \
          OPTIONAL_REG(is, cs), \
          OPTIONAL_REG(is, ss), \
          OPTIONAL_REG(is, eip), \
          OPTIONAL_REG(is, int_no)); \
} while (0);



void Pip_Debug_Putc(char c);
void Pip_Debug_PutWchar(uint16_t wchar);
void Pip_Debug_Puts(char *msg);
void Pip_Debug_PutDec(unsigned long n);
void Pip_Debug_PutHex(unsigned long n);
int printf(const char *format, ...);

#define RED() puts("\e[91m")
#define GREEN() puts("\e[92m")
#define BLUE() puts("\e[34m")
#define NOCOL() puts("\e[0m")

/**
 * \brief Defines the appropriate DEBUGRAW behavior.
 */
#define DEBUGRAW(a) krn_puts(a)
/**
 * \brief Defines the appropriate DEBUGHEX behavior.
 */
#define DEBUGHEX(a) puthex(a)
/**
 * \brief Defines the appropriate DEBUGDEC behavior.
 */
#define DEBUGDEC(a) putdec(a)
#endif
