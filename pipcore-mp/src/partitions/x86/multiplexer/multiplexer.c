/* Standard includes. */
#include "stdlib.h"
#include <stdint.h>

#include <pip/paging.h>
#include <pip/api.h>
#include <pip/vidt.h>
#include <pip/compat.h>
#include <pip/fpinfo.h>

#include "scheduler.h"
#include "isr.h"

#define ADDR_TO_MAP 0xA000000
#define MAX_PAGE 0x4000
#define NORMAL_MAX_PAGE 0x1000
#define SECURE_MAX_PAGE 0x1000
#define LOAD_ADDR 0x700000

extern void* _partition_freertos, _epartition_freertos;

extern void* _partition_secure, _epartition_secure;
void *partitionEntry;

extern void* _partition_normal, _epartition_normal;
void *normalPartitionEntry;

uint32_t _phy_dma_page, _v_dma_page;

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

void *create_partition(uint32_t load_addr, uint32_t base, uint32_t length, uint32_t free_mem_addr, uint32_t nb_free_pages)
{
    uint32_t offset;

    void *partitionEntry, *partpd, *partsh1, *partsh2, *partsh3;

    printf("Creating secure partition %x at %x, length %d\r\n", base,load_addr, length);
    partitionEntry = allocPage();
    partpd = allocPage();
    partsh1 = allocPage();
    partsh2 = allocPage();
    partsh3 = allocPage();
    printf(
            "Partition descriptor : %x \r\n\tpd : %x \r\n\tpartsh1 : %x \r\n\tpartsh2 : %x\r\n\tpartsh3 : %x\r\n",
            partitionEntry, partpd, partsh1, partsh2, partsh3);

    printf("Creating secure partition\r\n");
    if (!createPartition((uint32_t) partitionEntry, (uint32_t) partpd,
            (uint32_t) partsh1, (uint32_t) partsh2, (uint32_t) partsh3)) {
        printf("Failed to create secure partition\r\n");
        return 0;
    }

    printf("Mapping secure partition code...\r\n");

    for (offset = 0; offset < length; offset += 0x1000) {
        if (mapPageWrapper((uint32_t) (base + offset), partitionEntry,(uint32_t) (load_addr + offset))) {
            printf("Error during mapping %x into partition at \r\n",base+offset,load_addr+offset);
            return 0;
        }
    }

    uint32_t lastPage = load_addr + offset;
    printf("Partition mapped, last page : %x\r\n",lastPage);

    printf("Mapping stack... \r\n");
    uint32_t stack_off = 0;
    uint32_t stack_addr;
    for(stack_off = 0; stack_off <= 0x10000; stack_off+=0x1000)
    {
        stack_addr = (uint32_t)allocPage();
        if(mapPageWrapper((uint32_t)stack_addr, (uint32_t)partitionEntry, (uint32_t)0xB10000 + (stack_off)))
        // if(mapPageWrapper((uint32_t)stack_addr, (uint32_t)partitionEntry, (uint32_t)0xB10000 + (stack_off)))
        {
            printf("Couldn't map stack.\r\n");
            return 0;
        }
    }

    printf("Mapping additional memory for child\r\n");
    uint32_t page;
    pip_fpinfo * allocMem = (pip_fpinfo*) allocPage();

    allocMem->magic = FPINFO_MAGIC;
    allocMem->membegin = free_mem_addr;
    allocMem->memend = free_mem_addr+(nb_free_pages * 0x1000);


    int index;


    for(index = 0;index < nb_free_pages;index++){
        page = allocPage();
        if (mapPageWrapper((uint32_t)page, (uint32_t)partitionEntry, (uint32_t)( ADDR_TO_MAP+(index*0x1000))))
            printf("Failed to map additional memory %x at %x\r\n",page,ADDR_TO_MAP+index*0x1000);

    }

    printf("Index %d, nb_free_pages %d\r\n",index,nb_free_pages);
    if (mapPageWrapper(allocMem, (uint32_t)partitionEntry, (uint32_t)0xFFFFC000 )) {
        printf("Fail to map additional memory info\r\n");
        return 0;
    }
    printf("Done.\r\n");

    vidt_t* vidt = (vidt_t*) allocPage();
    printf("Task VIDT at %x\r\n",vidt);

    vidt->vint[0].eip = load_addr;
    vidt->vint[0].esp = 0xB10000 + 0x7000 - sizeof(uint32_t);
    vidt->flags = 0x1;
    printf("Partition stack is at 0x%x\r\n", vidt->vint[0].esp);

    if (mapPageWrapper((uint32_t)vidt, (uint32_t)partitionEntry, (uint32_t)0xFFFFF000)){
        printf("Failed to map Vidt\r\n");
        return 0;
    }
    return partitionEntry;
}

void main(uint32_t phy_dma_page, uint32_t v_dma_page)
{

    pip_fpinfo * bootinfo = (pip_fpinfo*)0xFFFFC000;
    printf("Hello I'm multiplexer\r\n");
    parse_bootinfo(bootinfo);

    uint32_t paging = initPaging((void*)bootinfo->membegin,(void*)bootinfo->memend);
    printf("Initlializing vcpus\r\n");
    initialize_vcpu();

    printf("phy_dma_page: 0x%x, v_dma_page: 0x%x\r\n", phy_dma_page, v_dma_page);
    uint32_t base=(uint32_t)&_partition_freertos, length=(uint32_t)&_epartition_freertos - base;
    void* freertos_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, MAX_PAGE);
    if (!freertos_partition){
        printf("Failed to initialize freertos partition\r\n");
        return;
    }
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

    bind_partition_2_vcpu(freertos_partition, 1);

    base=(uint32_t)&_partition_normal, length=(uint32_t)&_epartition_normal - base;
    void *normal_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, NORMAL_MAX_PAGE);
    if (!normal_partition){
        printf("Failed to initialize normal partition\r\n");
        return;
    }
    bind_partition_2_vcpu(normal_partition, 1);

    base=(uint32_t)&_partition_secure, length=(uint32_t)&_epartition_secure - base;
    void* secure_partition = create_partition(LOAD_ADDR, base, length, ADDR_TO_MAP, SECURE_MAX_PAGE);
    if (!secure_partition){
        printf("Failed to initialize Secure Partition\r\n");
        return;
    }
    bind_partition_2_vcpu(secure_partition, 1);


    printf("Initlializing timer isr\r\n");

    init_isr();

    printf("Start scheduling\r\n");

    schedule(0);

    for(;;);

    printf("multiplexer finished...\r\n");
    return;


}
