/*
 * debug_stdio_adaptor.c
 *
 *  Created on: 30 nov. 2017
 *      Author: hzgf0437
 */

#include <Quark_x1000_gen_defs.h>
#include "pip/debug.h"
#include "stdio.h"

#ifndef NULL
#define NULL   ((void *) 0)
#endif

void debug(const char*string){
	if(string != NULL){
		printf("%s", string);
	}
}

void debug1(const char *format, ...){
   va_list arg;

   va_start (arg, format);
   printf(format, arg);
   va_end (arg);
}
