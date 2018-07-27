#include <stdint.h>
#include <pip/paging.h>
#include <pip/debug.h>
#include <pip/api.h>

#define NULL 0

void *Pager_FirstFreePage = NULL;
static uint32_t nb_free_pages = 0;

/* Initializes paging, depending on the addresses given */
int Pip_InitPaging(void *begin, void *end)
{


	unsigned long p, b=(unsigned long)begin, e=(unsigned long)end, c=0;

	if (b >= e || (b & PGMASK) || (e & PGMASK))
		return -1;

	Pager_FirstFreePage = begin;

	for	( p = b + PGSIZE; p < e; p+=PGSIZE){


		*(void**)b = (void*)p;

		b = p;
		c += 1;
	}
	*(void**)b = (void*)0;
    nb_free_pages = c;

    Pip_Debug_Puts("LibPip2 : Paging initialization complete. ");
    Pip_Debug_PutDec(c);
    Pip_Debug_Puts(" pages available.\r\n");

	return 0;
}

/* Allocates a page */
void* Pip_AllocPage(void)
{
    if (nb_free_pages <=0){
        Pip_Debug_Puts("LibPip2: no more free pages available\r\n");
    }
    void* ret = Pager_FirstFreePage;
		if(!ret)
			return 0;

		Pager_FirstFreePage = *(void**)ret;
        nb_free_pages --;

		int j;
		for(j=0;j<PGSIZE;j++)
				((char*)ret)[j] = (char)0;

    // Pip_Debug_Puts("LibPip2: alloc page: 0x");
    // Pip_Debug_PutHex((unsigned long) ret);
    // Pip_Debug_Puts("\r\n");
    return ret;
}

/* Frees a page */
void Pip_FreePage(void* page)
{
    *(void**)page = Pager_FirstFreePage;
    Pager_FirstFreePage = page;
    nb_free_pages ++;
}
