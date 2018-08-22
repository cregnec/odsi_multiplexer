/* Standard includes. */
#include "stdlib.h"
#include <stdint.h>

#include <pip/paging.h>
#include <pip/debug.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/compat.h>
#include <pip/fpinfo.h>

void parse_bootinfo(pip_fpinfo* bootinfo)
{
    if(bootinfo->magic == FPINFO_MAGIC)
    LOGGER(LOG, "\tBootinfo seems to be correct.\r\n");
    else {
        LOGGER(ERROR, "\tBootinfo is invalid. Aborting.\r\n");
    }


    LOGGER(LOG, "\tAvailable memory starts at 0x%x and ends at 0x%x\r\n",(uint32_t)bootinfo->membegin, (uint32_t)bootinfo->memend);


    LOGGER(LOG, "\tPip revision %s\r\n",bootinfo->revision);
    return;
}


void main()
{

    pip_fpinfo * bootinfo = (pip_fpinfo*)0xFFFFC000;
    parse_bootinfo(bootinfo);
    LOGGER(WARNING, "Booting a partition that generates faults\r\n");
    int i = 0;
    for (;;){
        if (i%1000000 == 0){
            LOGGER(WARNING, "I'm looping and generating fault!!\r\n");
            *(uint32_t*) 0x10000 = 0;
            i = 1;
        }
        i++;
    }

}
