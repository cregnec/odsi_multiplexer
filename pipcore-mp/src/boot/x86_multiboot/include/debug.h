/*******************************************************************************/
/*  © Université Lille 1, The Pip Development Team (2015-2017)                 */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

/**
 * \file debug.h
 * \brief Include file for debugging output
 */


#ifndef __SCR__
#define __SCR__

#include <stdint.h>
#include <stdarg.h>
#include "mal.h"
#include "ial_defines.h"

/**
 * \brief Strings for debugging output.
 */
enum {
    CRITICAL =  0, //!< Critical output
    ERROR =     1, //!< Error output
    WARNING =   2, //!< Warning output
    INFO =      3, //!< Information output
    LOG =       4, //!< Log output
    TRACE =     5 //!< Annoying, verbose output
};
#define True 1
#define False 0

#ifdef PIPDEBUG

#ifndef LOGLEVEL
#define LOGLEVEL TRACE
#endif

/**
 * \brief Defines the appropriate DEBUGRAW behavior.
 */
#define DEBUGRAW(a) krn_puts(a)

/**
 * \brief Defines the appropriate DEBUG behavior.
 */
#define DEBUG(l,a,...) if(l<=LOGLEVEL){ kprintf(#l " [%s:%d] " a, __FILE__, __LINE__, ##__VA_ARGS__);}
#define IAL_DEBUG(l,a,...) if(l<=LOGLEVEL){ kprintf(#l " IAL [%s:%d] " a, __FILE__, __LINE__, ##__VA_ARGS__);}
/* #define DEBUG(l,a) { krn_puts(debugstr[l]); krn_puts("["); krn_puts(__FILE__); krn_puts(":"); putdec(__LINE__); krn_puts("] "); krn_puts(a);} */

/**
 * \brief Defines the appropriate DEBUGHEX behavior.
 */
#define DEBUGHEX(a) puthex(a)
/**
 * \brief Defines the appropriate DEBUGDEC behavior.
 */
#define DEBUGDEC(a) putdec(a)
/**
 * \fn dumpRegs(int_ctx_t* is, uint32_t outputLevel)
 * \brief Dumps the registers of a saved interrupt context onto the serial output.
 * \param is Interrupted state
 * \param outputLevel Serial log debugging output level
 */
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
    if(isKernel(OPTIONAL_REG(is, cs))) \
    { \
        DEBUG(outputLevel, "               cs=%x, eip=%x, int=%x\r\n", \
              OPTIONAL_REG(is, cs), \
              OPTIONAL_REG(is, eip), \
              OPTIONAL_REG(is, int_no)); \
    } else { \
        DEBUG(outputLevel, "               cs=%x, ss=%x, eip=%x, int=%x\r\n", \
              OPTIONAL_REG(is, cs), \
              OPTIONAL_REG(is, ss), \
              OPTIONAL_REG(is, eip), \
              OPTIONAL_REG(is, int_no)); \
    } \
} while (0);

#else
/**
 * \brief Defines the appropriate DEBUG behavior.
 */
#define DEBUG(...)
#define DEBUGRAW(...)
/**
 * \brief Defines the appropriate DEBUGHEX behavior.
 */
#define DEBUGHEX(...)
/**
 * \brief Defines the appropriate DEBUGDEC behavior.
 */
#define DEBUGDEC(...)

#define dumpRegs(...)

#endif

void krn_puts(char *c);
void kaput(char c);
void puthex(int n);
void putdec(int n);

void counter_update(uint32_t begin);
void display_time();

void kprintf(char *fmt, ...);

#define BENCH_BEGIN counter_update(1)
#define BENCH_END {counter_update(0); DEBUG(TRACE, "Benchmark lasted "); display_time();}
#endif
