/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


/*
 * The library functions not provided by libgcc for freestanding environments.
 * The implementation of the functions in this file have made NO attempt
 * whatsoever to be optimised!
 */

#warning The functions in this file are very basic, and not optimised.

#include <stdint.h>
#include <stddef.h>

/*-----------------------------------------------------------*/

void *memcpy( void *pvDest, const void *pvSource, size_t xBytes )
{
/* The compiler used during development seems to err unless these volatiles are
included at -O3 optimisation.  */
volatile unsigned char *pcDest = ( volatile unsigned char * ) pvDest, *pcSource = ( volatile unsigned char * ) pvSource;
size_t x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
	if( pvDest != pvSource )
	{
		for( x = 0; x < xBytes; x++ )
		{
			pcDest[ x ] = pcSource[ x ];
		}
	}

	return pvDest;
}
/*-----------------------------------------------------------*/

void *memset( void *pvDest, int iValue, size_t xBytes )
{
/* The compiler used during development seems to err unless these volatiles are
included at -O3 optimisation.  */
volatile unsigned char * volatile pcDest = ( volatile unsigned char * volatile ) pvDest;
volatile size_t x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
	for( x = 0; x < xBytes; x++ )
	{
		pcDest[ x ] = ( unsigned char ) iValue;
	}

	return pvDest;
}
/*-----------------------------------------------------------*/

int memcmp( const void *pvMem1, const void *pvMem2, unsigned long ulBytes )
{
const volatile unsigned char *pucMem1 = pvMem1, *pucMem2 = pvMem2;
register unsigned long x;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */
    for( x = 0; x < ulBytes; x++ )
    {
        if( pucMem1[ x ] != pucMem2[ x ] )
        {
            break;
        }
    }

    return ulBytes - x;
}
/*-----------------------------------------------------------*/

int strcmp( const char *pcString1, const char *pcString2 )
{
volatile int iReturn, iIndex = 0;

	/* Extremely crude standard library implementations in lieu of having a C
	library. */

	while( ( pcString1[ iIndex ] != 0x00 ) && ( pcString2[ iIndex ] != 0x00 ) )
	{
		iIndex++;
	}

	if( ( pcString1[ iIndex ] == 0x00 ) && ( pcString2[ iIndex ] == 0x00 ) )
	{
		iReturn = 0;
	}
	else
	{
		iReturn = ~0;
	}

	return iReturn;
}

/**
 * strlen: measure length of the string
 * @param str
 * @return length of the string
 */
unsigned long strlen( const char* str ){
	unsigned long i = 0;

	while (str[i] != '\0')
	{
		i++;
	}

	return i;
}

/**
 * reverse:  reverse string s in place
 * @param s string
 */
void reverse(char s[])
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/**
 * safe_reverse:  reverse string s in place
 * @param s string
 * @param string_size the size of the string s
 */
void safe_reverse(char s[], int string_size)
{
	int i, j;
	char c;

	for (i = 0, j = string_size - 1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/**
 * myatoi: convert string str to int
 * @param str
 * @return converted int
 */
int atoi(const char *str)
{
    int res = 0; // Initialize result

    // Iterate through all characters of input string and
    // update result
    for (int i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';

    // return result.
    return res;
}

/**
 * itoa:  convert n to characters in s
 * @param n number to be converted
 * @param s output string
 * @param base base for conversion
 * @result string
 */
char* itoa(int n, char s[], int base)
{
	int i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do {       /* generate digits in reverse order */
		if( base > 10 && ((n % base) > 10) )
			s[i++] = n%base -10 + 'A';
		else
			s[i++] = n % base + '0';   /* get next digit */
	} while ((n /= base) > 0);     /* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
	return s;
}

/**
 * safe_itoa:  convert n to characters in s
 * @param n number to be converted
 * @param s output string
 * @param base base for conversion
 * @param string_max_length length of character array, param s
 * @result length of string
 */
int safe_itoa(int n, char s[], int base, int string_max_length)
{
	int i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	i = 0;
	do
	{       /* generate digits in reverse order */
		if(i > string_max_length - 1)
		{
			return 0;
		}

		if( base > 10 && ((n % base) > 10) )
		{
			s[i++] = n%base -10 + 'A';
		}
		else
		{
			s[i++] = n % base + '0';   /* get next digit */
		}
	}
	while ((n /= base) > 0);     /* delete it */

	if (sign < 0)
	{
		if(i > string_max_length - 1)
		{
			return 0;
		}

		s[i++] = '-';
	}

	if(i > string_max_length - 1)
	{
		return 0;
	}

	s[i] = '\0';
	safe_reverse(s, i);

	return i;
}

/**
 * mystrcat: concatenate string dest and src in dest
 * @param dest
 * @param src
 * @return concatenated string
 */
char* strcat(char *dest, const char *src){
	int i,j;
	for (i = 0; dest[i] != '\0'; i++)
	{
		;
	}
	for (j = 0; src[j] != '\0'; j++)
	{
		dest[i+j] = src[j];
	}
	dest[i+j] = '\0';
	return dest;
}
