/* Standard includes. */
#include "stdlib.h"
#include <stdint.h>

#include <pip/paging.h>
#include <pip/debug.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/compat.h>
#include <pip/fpinfo.h>

#include "scheduler.h"
#include "isr.h"

#define ADDR_TO_MAP 0xA000000
#define MAX_PAGE 0x4000
#define FAULT_MAX_PAGE 0x1000
#define SECURE_MAX_PAGE 0x1000
#define LOAD_ADDR 0x700000

extern void *_partition_freertos, *_epartition_freertos;

extern void *_partition_secure, *_epartition_secure;

extern void *_partition_fault, *_epartition_fault;

uint32_t _phy_dma_page, _v_dma_page;

void parse_bootinfo(pip_fpinfo* bootinfo)
{
    if(bootinfo->magic == FPINFO_MAGIC)
        LOGGER(TRACE, "\tBootinfo seems to be correct.\r\n");
    else {
        LOGGER(ERROR, "\tBootinfo is invalid. Aborting.\r\n");
    }


    LOGGER(TRACE, "\tAvailable memory starts at 0x%x and ends at 0x%x\r\n",(uint32_t)bootinfo->membegin, (uint32_t)bootinfo->memend);


    LOGGER(TRACE, "\tPip revision %s\r\n",bootinfo->revision);
    return;
}

void *create_partition(uint32_t load_addr, uint32_t base, uint32_t length, uint32_t free_mem_addr, uint32_t nb_free_pages)
{
    uint32_t offset;

    void *partitionEntry, *partpd, *partsh1, *partsh2, *partsh3;

    LOGGER(LOG, "Creating partition %x at %x, length %d\r\n", base,load_addr, length);
    partitionEntry = allocPage();
    partpd = allocPage();
    partsh1 = allocPage();
    partsh2 = allocPage();
    partsh3 = allocPage();
    LOGGER(LOG,
            "Partition descriptor : %x \r\n\tpd : %x \r\n\tpartsh1 : %x \r\n\tpartsh2 : %x\r\n\tpartsh3 : %x\r\n",
            partitionEntry, partpd, partsh1, partsh2, partsh3);

    LOGGER(LOG, "Creating partition\r\n");
    if (!createPartition((uint32_t) partitionEntry, (uint32_t) partpd,
            (uint32_t) partsh1, (uint32_t) partsh2, (uint32_t) partsh3)) {
        LOGGER(ERROR, "Failed to create partition\r\n");
        return 0;
    }

    LOGGER(LOG, "Mapping partition code...\r\n");

    for (offset = 0; offset < length; offset += 0x1000) {
        if (mapPageWrapper((uint32_t) (base + offset), (uint32_t)partitionEntry,(uint32_t) (load_addr + offset))) {
            LOGGER(ERROR, "Error during mapping %x into partition at \r\n",base+offset,load_addr+offset);
            return 0;
        }
    }

    uint32_t lastPage = load_addr + offset;
    LOGGER(LOG, "Partition mapped, last page : %x\r\n",lastPage);

    LOGGER(LOG, "Mapping stack... \r\n");
    uint32_t stack_off = 0;
    uint32_t stack_addr;
    for(stack_off = 0; stack_off <= 0x10000; stack_off+=0x1000)
    {
        stack_addr = (uint32_t)allocPage();
        if(mapPageWrapper((uint32_t)stack_addr, (uint32_t)partitionEntry, (uint32_t)0xB10000 + (stack_off)))
        // if(mapPageWrapper((uint32_t)stack_addr, (uint32_t)partitionEntry, (uint32_t)0xB10000 + (stack_off)))
        {
            LOGGER(ERROR, "Couldn't map stack.\r\n");
            return 0;
        }
    }

    LOGGER(LOG, "Mapping additional memory for child\r\n");
    void* page;
    pip_fpinfo * allocMem = (pip_fpinfo*) allocPage();

    allocMem->magic = FPINFO_MAGIC;
    allocMem->membegin = free_mem_addr;
    allocMem->memend = free_mem_addr+(nb_free_pages * 0x1000);


    int index;


    for(index = 0;index < nb_free_pages;index++){
        page = allocPage();
        if (mapPageWrapper((uint32_t)page, (uint32_t)partitionEntry, (uint32_t)( ADDR_TO_MAP+(index*0x1000))))
            LOGGER(ERROR, "Failed to map additional memory %x at %x\r\n",page,ADDR_TO_MAP+index*0x1000);

    }

    LOGGER(LOG, "Index %d, nb_free_pages %d\r\n",index,nb_free_pages);
    if (mapPageWrapper((uint32_t)allocMem, (uint32_t)partitionEntry, (uint32_t)0xFFFFC000 )) {
        LOGGER(ERROR, "Fail to map additional memory info\r\n");
        return 0;
    }
    LOGGER(LOG, "Done.\r\n");

    vidt_t* vidt = (vidt_t*) allocPage();
    LOGGER(LOG, "Task VIDT at %x\r\n",vidt);

    vidt->vint[0].eip = load_addr;
    vidt->vint[0].esp = 0xB10000 + 0x7000 - sizeof(uint32_t);
    vidt->flags = 0x1;
    LOGGER(LOG, "Partition stack is at 0x%x\r\n", vidt->vint[0].esp);

    if (mapPageWrapper((uint32_t)vidt, (uint32_t)partitionEntry, (uint32_t)0xFFFFF000)){
        LOGGER(ERROR, "Failed to map Vidt\r\n");
        return 0;
    }
    return partitionEntry;
}

void main(uint32_t phy_dma_page, uint32_t v_dma_page)
{

    pip_fpinfo * bootinfo = (pip_fpinfo*)0xFFFFC000;
    LOGGER(INFO, "Booting\r\n");
    parse_bootinfo(bootinfo);

    uint32_t paging = initPaging((void*)bootinfo->membegin,(void*)bootinfo->memend);
    LOGGER(LOG, "Initlializing vcpus\r\n");
    initialize_vcpu();

    LOGGER(LOG, "phy_dma_page: 0x%x, v_dma_page: 0x%x\r\n", phy_dma_page, v_dma_page);
    uint32_t base=(uint32_t)&_partition_freertos, length=(uint32_t)&_epartition_freertos - base;
    void* freertos_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, MAX_PAGE);
    if (!freertos_partition){
        LOGGER(ERROR, "Failed to initialize freertos partition\r\n");
        return;
    }
    LOGGER(INFO, "Create FREERTOS partition: 0x%x\r\n", (uint32_t)freertos_partition);

    _phy_dma_page = phy_dma_page;
    _v_dma_page = v_dma_page;
    mapPageWrapper((uint32_t) 0xE00A1000, (uint32_t)freertos_partition, 0xE00A1000);
    mapPageWrapper((uint32_t) 0x9000F000, (uint32_t)freertos_partition, 0x9000F000);
    mapPageWrapper((uint32_t) 0x9000E000, (uint32_t)freertos_partition, 0x9000E000);
    mapPageWrapper((uint32_t) 0xE00AA000, (uint32_t)freertos_partition, 0xE00AA000);
    mapPageWrapper((uint32_t) 0x90006000, (uint32_t)freertos_partition, 0x90006000);
    mapPageWrapper((uint32_t) 0x90007000, (uint32_t)freertos_partition, 0x90007000);

    // mapPageWrapper((uint32_t)EC_BASE,(uint32_t)freertos_partition,EC_BASE);
    // mapPageWrapper((uint32_t)UART_MMIO_BSE,(uint32_t)freertos_partition,UART_MMIO_BSE);
    // mapPageWrapper((uint32_t)UART_PCI_BSE,(uint32_t)freertos_partition,UART_PCI_BSE);


    mapPageWrapper(v_dma_page, (uint32_t)freertos_partition, v_dma_page);

    bind_partition_2_vcpu((uint32_t)freertos_partition, 1);

    base=(uint32_t)&_partition_fault, length=(uint32_t)&_epartition_fault - base;
    void *fault_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, FAULT_MAX_PAGE);
    if (!fault_partition){
        LOGGER(ERROR, "Failed to initialize fault partition\r\n");
        return;
    }
    LOGGER(INFO, "Create FAULT partition: 0x%x\r\n", (uint32_t)fault_partition);
    bind_partition_2_vcpu((uint32_t)fault_partition, 1);

    base=(uint32_t)&_partition_secure, length=(uint32_t)&_epartition_secure - base;
    void* secure_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, SECURE_MAX_PAGE);
    if (!secure_partition){
        LOGGER(ERROR, "Failed to initialize Secure Partition\r\n");
        return;
    }
    LOGGER(INFO, "Create SECURE partition: 0x%x\r\n", (uint32_t)secure_partition);
    bind_partition_2_vcpu((uint32_t)secure_partition, 1);


    LOGGER(LOG, "Initlializing timer isr\r\n");

    init_isr();

    LOGGER(INFO, "Start scheduling\r\n");

    schedule(0);

    for(;;);

    LOGGER(INFO, "Finished...\r\n");
    return;


}
