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
 * \file mmu.c
 * \brief MMU early-boot configuration
 */

#include "mmu.h"
#include "multiboot.h"
#include <stdint.h>
#include "debug.h"
#include "mal.h"
#include "structures.h"
#include "fpinfo.h"
#include "git.h"
#include <libc.h>
#include "galileo-support.h"

page_directory_t *kernelDirectory=0; //!< The kernel's page directory

uint32_t maxPages = 0; //!< The maximal amount of pages available
uint32_t allocatedPages = 0; //!< The current allocated amount of pages
uint32_t ramEnd = 0; //!< End of memory
uint32_t pageCount = 0;
uint32_t phy_dma_page, v_dma_page;
// Defined in libc.c
extern uint32_t placement_address; //!< Placement address, this should be unused.
uint32_t test = 0;

uint32_t root_stack_addr;

/*!	\fn void mapPageC(uintptr_t pd, uintptr_t p, uintptr_t v)
	\brief add a mapping to a physical page p into a given page directory pd and a virtual address v.
	\param pd a page directory
	\param v a virtual address
	\param p a physical page
	\post p is mapped in pd at v
 */
void mapPageC(uintptr_t pd, uintptr_t p, uintptr_t v, uint8_t user)
{
	/*
	 * First of all : get Page Directory Entry.
	 */
	uintptr_t pdIdx = (v & 0xFFC00000) >> 22;
	uintptr_t ptEntry = (v >> 12) & 0x000003FF;
	page_table_t *pt;
	pt = (page_table_t*)(((page_directory_t*)pd)->tablesPhysical[pdIdx]);

	/*
	 * Let's just get the Page Table address, shall we ?
	 */
	pt = (page_table_t*)((uintptr_t)pt & 0xFFFFF000);

	/*
	 * Check if we have an existing Page Table here.
	 * If not, create it.
	 */
	if(!pt)
	{
		/* pt = *((uintptr_t*)list);
		 ((page_directory_t*)pd)->tablesPhysical[pd_idx] = (uintptr_t)pt | 0x7; // Page Table is present, read & write, user-mode.  */
		return;
	}

	/*
	 * Now we should have a Page Table. Find the corresponding Page Table Entry.
	 */
	page_table_entry_t pte = pt->pages[ptEntry];


	/*
	 * Configure it, and we're done.
	 * pte.pat must be to 0
	 */
	pte.present = 1;
	pte.rw = 1;
	pte.user = user;
	pte.pat = 0;
	pte.frame = (uintptr_t)p >> 12 ;


	pt->pages[ptEntry] = pte;

}


/*!
 * \fn void prepareC(uintptr_t pd, uintptr_t v, uintptr_t* page_list)
 * \brief Prepares a Page Directory to receive a mapping by inserting the according Page Table.
 *
 * \param pd a page directory
 * \param v a virtual address
 * \param page_list
 */
void prepareC(uintptr_t pd, uintptr_t v, uintptr_t* pageList)
{
	uintptr_t pdIdx = (v & 0xFFC00000) >> 22;
	page_table_t *pt;
	pt = (page_table_t*)(((page_directory_t*)pd)->tablesPhysical[pdIdx]);

	pt = (page_table_t*)((uintptr_t)pt & 0xFFFFF000);

	if(!pt)
	{
		pt = (page_table_t*)(*((uintptr_t*)pageList));
		((page_directory_t*)pd)->tablesPhysical[pdIdx] = (uintptr_t)pt | 0x7;

		/* Now update mappings into the given CR3 */
		mapPageC(pd, (uintptr_t)pt, (uintptr_t)pt, 0);
	}
	return;
}

/*!	\fn uintptr_t pageCountMapPageC(uintptr_t pd, uintptr_t vaddr)
	\brief tests if there is a page table in pd at vaddr
	\param pd a page directory
	\param uintptr_t vaddr
	\return 1 if there is no page table at position pd_idx, 0 else
 */
uint32_t pageCountMapPageC(uintptr_t pd, uintptr_t vaddr)
{
	uintptr_t pdIdx = (vaddr & 0xFFC00000) >> 22;
	page_table_t *pt = (page_table_t*)(((page_directory_t*)pd)->tablesPhysical[pdIdx]);
	pt = (page_table_t*)((uintptr_t)pt & 0xFFFFF000);

	if(!pt)
		return 1;
	else
		return 0;
}

/**
 * \fn void mapPageWrapper(page_directory_t* dir, uint32_t paddr, uint32_t vaddr)
 * \brief Wraps the MAL calls into a single function to map a vaddr into a given page directory
 * \param dir The target page directory
 * \param paddr The source physical address
 * \param vaddr The target virtual address
 * \param user user/kernel access bit
 */
void mapPageWrapper(page_directory_t* dir, uint32_t paddr, uint32_t vaddr, uint8_t user)
{
    uint32_t pdAddr = (uint32_t)dir;
    uint32_t pc = pageCountMapPageC(pdAddr, vaddr);
    uint32_t list[1];
    if(pc == 1) {
	// Allocate entry for prepare, and shadowed pages
        list[0] = (uint32_t)allocPage();
    }

    prepareC(pdAddr, vaddr, list);
    mapPageC(pdAddr, paddr, vaddr, user);
}
uint32_t started = 0;
/**
 * \fn void initFreePageList(multiboot_memory_map_t* mmap)
 * \brief Initializes the free page list, given a multiboot-compliant memory map
 * \param mmap The memory map given by Multiboot
 */
void initFreePageList(uintptr_t base, uintptr_t length)
{
    extern uint32_t end;


	DEBUG(CRITICAL, "Adding memory region %x length %x", base, length);
	extern uintptr_t __end;



	if(base >= 0x100000 && length > 1*PAGE_SIZE)
	{
		uint32_t i;

		/* Add each page of free area */
		for(i = base ; i < base + length - 0x1000; i+=0x1000)
		{

			/* Ignore kernel area */
			if(i > (uint32_t)&__end) {
				if(!started){
					started = 1;
					firstFreePage = (uint32_t*) i;
				}

				*(uint32_t*)i = (uint32_t)firstFreePage; /* Add current page as head of list */
				firstFreePage = (uint32_t*)i;
				pageCount++;
//				if(!((uint32_t)i % 0x100000))
//					DEBUG(TRACE,"%x",i);

			}
		}

		DEBUG(CRITICAL, "Added memory region to page allocator, first page %x, last page at %x, %d pages", firstFreePage, base,length/PAGE_SIZE);
		maxPages += length;
		ramEnd = i;
        DEBUG(CRITICAL, "Ram ends at %x\r\n and the last page if reserved for DMA", ramEnd);
	} else {
		DEBUG(CRITICAL, "Not adding low-memory area");
	}
}
/**
 * \fn uint32_t* allocPage()
 * \brief Unsafe page allocator. Allocated a page.
 * \return Virtual address to a free page.
 */
uint32_t* allocPage()
{
    if (allocatedPages >= pageCount){
        DEBUG(CRITICAL, "No more free memory available");
    }
    uint32_t* res = firstFreePage;
    firstFreePage = (uint32_t*)(*res);

    allocatedPages++;

    return res;
}

/**
 * \fn void freePage(uint32_t *page)
 * \brief Frees a page in a really unsafe way.
 * \param page The page to free.
 */
void freePage(uint32_t *page)
{
    *page = (uint32_t)firstFreePage;
    firstFreePage = (uint32_t*)page;

    allocatedPages--;
}

/**
 * \fn void dumpMmap(uint32_t *mmap_ptr, uint32_t len)
 * \brief Despite its unexplicit name, this function initializes the physical memory, preparing the page allocator as well.
 * \param mmap_ptr Pointer to a multiboot-compliant memory map
 * \param len Length of the memory map structure
 */
void dumpMmap(uint32_t *mmap_ptr, uint32_t len)
{
    // Gets size of structure
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mmap_ptr;
    uint32_t num = 1;

    multiboot_memory_map_t* ram = NULL;
    extern uint32_t code;

    // Parse each entry
    while((uint32_t*)mmap < (uint32_t*)((uint32_t)mmap_ptr + len) && mmap->size > 0)
    {
		DEBUG(CRITICAL, "region %d, addr %x, length %x", num, mmap->base_addr_low, mmap->length_low);
        switch(mmap->type){
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            DEBUG(CRITICAL, "\tACPI_RECLAIMABLE");
            break;
        case MULTIBOOT_MEMORY_AVAILABLE:
            DEBUG(CRITICAL, "\tAVAILABLE");
            initFreePageList(mmap->base_addr_low, mmap->length_low);
            break;
        case MULTIBOOT_MEMORY_BADRAM:
            DEBUG(CRITICAL, "\tBADRAM");
            break;
        case MULTIBOOT_MEMORY_NVS:
            DEBUG(CRITICAL, "\tNVS");
            break;
        case MULTIBOOT_MEMORY_RESERVED:
            DEBUG(CRITICAL, "\tRESERVED");
            break;
        default:
            DEBUG(CRITICAL, "\tUNKNOWN");
            break;
        }



        num++;
        mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );

    }

    DEBUG(CRITICAL,"Amount of page available : %d",maxPages/PAGE_SIZE);
}

/* Marks the whole kernel area as global, preventing TLB invalidations */
void mark_kernel_global()
{
	#define GLOBAL_BIT (1 << 8)
	uint32_t pd_idx = kernelIndex();
	uint32_t kern_pt = readTableVirtual((uint32_t)kernelDirectory, pd_idx);
	uint32_t i = 0;
	/* Mark each entry of kernel PT as global */
	for(i = 0; i < 1024; i++)
	{
		uint32_t kern_pte = readTableVirtual((uint32_t)kern_pt, i);
		if(kern_pte) {
			writeTableVirtual((uint32_t)kern_pt, i, kern_pte | GLOBAL_BIT);
		}
	}

	/* Mark kernel PT as global */
	writeTableVirtual((uint32_t)kernelDirectory, pd_idx, kern_pt | GLOBAL_BIT);

	return;
}

/**
 * \fn void initMmu()
 * \brief Initializes the MMU, creating the kernel's page directory and switching to it.
 */
void initMmu()
{

    /* Create the Kernel Page Directory */
    kernelDirectory = (page_directory_t*)allocPage(); // kmalloc(sizeof(page_directory_t));
	DEBUG(TRACE, "Kernel directory is at %x", kernelDirectory);
    memset(kernelDirectory, 0, sizeof(page_directory_t));


    DEBUG(TRACE,"Mapping the kernel space");
    /* Map the kernel space */
    extern uint32_t end, __code, __kernel_end, __multiplexer, __krnstack;
    uint32_t curAddr = (uint32_t) &__code;
    DEBUG(TRACE,"Map kernel, stack up to root partition");
    /* Map kernel, stack up to root partition */
    while(curAddr < (uint32_t)(/* &end */ /* RAM_END */&__kernel_end))
    {
        mapPageWrapper(kernelDirectory, curAddr, curAddr, 0);
        curAddr += PAGE_SIZE;
    }

    mapPageWrapper(kernelDirectory, (uint32_t) &__krnstack, (uint32_t) &__krnstack, 0);
	DEBUG(TRACE, "Kernel directory is at %x", kernelDirectory);


    DEBUG(TRACE,"Mapping the root partition in userland");
	/* Map root partition in userland */
	curAddr = (uint32_t)&__multiplexer;
	while(curAddr <= (uint32_t)(&end /* RAM_END */ /* 0xFFFFE000 */))
	{
		mapPageWrapper(kernelDirectory, curAddr, curAddr, 1);
		curAddr += PAGE_SIZE;
	}

    mapPageWrapper(kernelDirectory, 0xB8000, 0xB8000, 1);

    DEBUG(TRACE,"pseudo-prepare kernel directory, removing page table from free page list");
    /* First, pseudo-prepare kernel directory, removing potential page tables from free page list */
    uint32_t j = 0;
    for(j = 0; j < 0xFFFFF000; j+=0x1000)
    {
        uint32_t pc = pageCountMapPageC((uintptr_t)kernelDirectory, j);
        uint32_t list[1];
        if(pc == 1) {
            list[0] = (uint32_t)allocPage();
            memset((void*)list[0], 0x0, PAGE_SIZE);
            prepareC((uintptr_t)kernelDirectory, j, list);
        }
    }
	mark_kernel_global();


    DEBUG(TRACE,"Map a linear memory space using page allocator");
	/* Map a linear memory space using page allocator \o/ */
	curAddr = (uint32_t)&end;
	uint32_t pg;

    DEBUG(TRACE,"Map first partition info as user-accessible");
    /* Map first partition info as user-accessible */
    extern pip_fpinfo* fpinfo;

	fpinfo = (pip_fpinfo*)allocPage();
	DEBUG(TRACE, "Allocated FpInfo to %x", fpinfo);
    uintptr_t fpInfoBegin = (uintptr_t)fpinfo;

	mapPageWrapper(kernelDirectory, (uint32_t)fpInfoBegin, (uint32_t)fpInfoBegin, 1);
    DEBUG(TRACE,"Map the first free page into our kernel's virtual address space");
    // Map the first free page into our kernel's virtual address space
    mapPageWrapper(kernelDirectory, (uint32_t)firstFreePage, (uint32_t)firstFreePage, 1);

	/* TODO : check the correctness of this. The initial state of the system HAS to be correct, this is just a hackfix right now */
    DEBUG(TRACE,"Build environment for the main partition");
    /* Now we have to build a proper environment for main partition */
	uint32_t* partitionDescriptor = allocPage(); // Partition descriptor
	uint32_t* sh1 = allocPage();
    uint32_t* sh2 = allocPage();
    uint32_t* sh3 = allocPage(); // Allocate shadow list
    *sh3 = 2; // TODO: fill sh3 with each indirection table

	/* At this point we're still in physical memory, so we can use writeVirtual, which won't tamper with CR0/CR3 configuration */

    for(uint32_t i = 1; i < 1024; i++)
    {
        uint32_t* ptsh1 = allocPage();
		memset(ptsh1, 0x0, PAGE_SIZE);

        uint32_t* ptsh2 = allocPage();
		memset(ptsh2, 0x0, PAGE_SIZE);

        //*(sh1 + (uint32_t)(i * sizeof(uint32_t))) = (uint32_t)ptsh1;
        //*(sh2 + (uint32_t)(i * sizeof(uint32_t))) = (uint32_t)ptsh2;
		writeTableVirtualNoFlags((uint32_t)sh1, i, (uint32_t)ptsh1);
		writeTableVirtualNoFlags((uint32_t)sh2, i, (uint32_t)ptsh2);
    }

	DEBUG(TRACE, "Page allocation ends at %x, multiplexer descriptor is %x", firstFreePage, partitionDescriptor);

	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 0, (uintptr_t)partitionDescriptor); // Store descriptor into descriptor
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 1, (uintptr_t)partitionDescriptor);
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 2, (uintptr_t)kernelDirectory); // Store page directory into descriptor
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 3, (uintptr_t)kernelDirectory);
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 4, (uintptr_t)sh1); // Store shadow 1 into descriptor
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 5, (uintptr_t)sh1);
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 6, (uintptr_t)sh2); // Store shadow 2 into descriptor
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 7, (uintptr_t)sh2);
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 8, (uintptr_t)sh3); // Store shadow 3 into descriptor
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 9, (uintptr_t)sh3);
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 10, (uintptr_t)0xFFFFFFFF); // Store IO mask into descriptor, allowing any IO
	writeTableVirtualNoFlags((uintptr_t)partitionDescriptor, 11, (uintptr_t)0); //parent paddr: null for multiplexer

	// Current partition is now our descriptor
	extern uint32_t current_partition;
	current_partition = (uint32_t)partitionDescriptor;
	updateRootPartition((uintptr_t)partitionDescriptor);

	DEBUG(TRACE,"Create fake IDT");
	// Create fake IDT at 0xFFFFF000
	uint32_t* virt_intv = allocPage();
	mapPageWrapper(kernelDirectory, (uint32_t)virt_intv, 0xFFFFF000, 1);
	// mapPageWrapper(kernelDirectory, (uint32_t)0xB8000, 0xFFFFE000, 1);

	// Fill Virtu. IDT info
	*virt_intv = (uint32_t)(&__multiplexer); // Multiplexer load addr

	DEBUG(TRACE, "Building linear memory space");

    DEBUG(TRACE,"Build the multiplexer stack");
	/* Build a multiplexer stack */
    mapPageWrapper(kernelDirectory, (uint32_t)allocPage(), 0xFFFFD000, 1);
	mapPageWrapper(kernelDirectory, (uint32_t)allocPage(), 0xFFFFE000, 1);
    root_stack_addr = 0xFFFFF000 - 4;

    DEBUG(TRACE,"Map first partition info");
	/* Map first partition info */
	mapPageWrapper(kernelDirectory, (uint32_t)fpinfo, 0xFFFFC000, 1);







	/* We should be done with page allocation and stuff : the remaining pages should be available as memory for the partition */
	/* First prepare all pages : pages required for prepare should be deleted from free page list */
	while((pg = (uint32_t)allocPage()) && allocatedPages < pageCount) {
		mapPageC((uintptr_t)kernelDirectory, pg, curAddr, 1);
		curAddr += 0x1000;
	}



	/* Fix first partition info */
	fpinfo->membegin = (uint32_t)&end;
	fpinfo->memend = curAddr;
	fpinfo->magic = FPINFO_MAGIC;
	strcpy(fpinfo->revision, GIT_REVISION);

	/* At this point, page allocator is empty. */
	DEBUG(TRACE, "Partition environment is ready, membegin=%x, memend=%x", fpinfo->membegin, fpinfo->memend);

	/* Our Kernel Page Directory is created, write its address into CR3. */

		/* Map UART */


	DEBUG(WARNING,"Mapping MMIO into root partition\r\n");


	int index;
	for(index = 0;index <= 0x2000000;index+=0x1000){
			mapPageWrapper(kernelDirectory,(uint32_t)(0xe0000000+index),(uint32_t)(0xe0000000+index),1);
			mapPageWrapper(kernelDirectory,(uint32_t)(0x90000000+index),(uint32_t)(0x90000000+index),1);
	}

    phy_dma_page = ramEnd;
    v_dma_page = 0xFFFEC000;
    mapPageWrapper(kernelDirectory, phy_dma_page, v_dma_page, 1);

	activate((uint32_t)kernelDirectory);
}
