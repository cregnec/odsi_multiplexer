/* Standard includes. */
#include "stdlib.h"
#include "stdio.h"
#include <stdint.h>

#include <pip/paging.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/compat.h>
#include <pip/fpinfo.h>


void parse_bootinfo(pip_fpinfo* bootinfo)
{
    if(bootinfo->magic == FPINFO_MAGIC)
    printf("\tBootinfo seems to be correct.\r\n");
    else {
        printf("\tBootinfo is invalid. Aborting.\r\n");
    }


    printf("\tAvailable memory starts at 0x%x and ends at 0x%x\r\n",(uint32_t)bootinfo->membegin, (uint32_t)bootinfo->memend);


    printf("\tPip revision %s\r\n",bootinfo->revision);
    return;
}


void main()
{

    pip_fpinfo * bootinfo = (pip_fpinfo*)0xFFFFC000;
    printf("Hello I'm from secure partition\r\n");
    parse_bootinfo(bootinfo);

    int i = 0;
    for (;;){
        if (i%1000000 == 0){
            printf("I'm looping\r\n");
            i = 1;
        }
        i ++;
    }

}
