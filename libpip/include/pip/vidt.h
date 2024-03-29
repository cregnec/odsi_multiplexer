#ifndef __VIDT__
#define __VIDT__

#ifdef __ASSEMBLY__
#define INTERRUPT_HANDLER(a, n) \
		.globl a; \
				.extern vcli; \
				.align 4; \
				a : \
					push %ecx; \
					push %ebx; \
					push %eax; \
					.extern n; \
					call n;
#else

#include <stdint.h>

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

#ifndef __pack
#define __pack __attribute__((packed))
#endif


/**
 * \struct registers
 * \brief Registers structure for x86
 */
typedef struct  __pack pushad_regs_s
{
	uint32_t edi; //!< General register EDI
	uint32_t esi; //!< General register ESI
	uint32_t ebp; //!< EBP
	uint32_t esp; //!< Stack pointer
	uint32_t ebx; //!< General register EBX
	uint32_t edx; //!< General register EDX
	uint32_t ecx; //!< General register ECX
	uint32_t eax; //!< General register EAX
} pushad_regs_t;

typedef struct __pack vidt_int_ctx_s
{
	uint32_t eip;
	uint32_t pipflags;
	uint32_t eflags;
	pushad_regs_t regs;
	uint32_t valid;
	uint32_t nfu[4];
} vidt_int_ctx_t;

/**
 * \struct int_stack_s
 * \brief Stack context from interrupt/exception
 */
typedef const struct int_stack_s
{
    pushad_regs_t regs;//!< Interrupt handler saved regs
    uint32_t int_no; //!< Interrupt number
    uint32_t err_code; //!< Interrupt error code
    uint32_t eip; //!< Execution pointer
    uint32_t cs; //!< Code segment
    uint32_t eflags; //!< CPU flags
    /* only present when we're coming from userland */
    uint32_t useresp; //!< User-mode ESP
    uint32_t ss; //!< Stack segment
} int_ctx_t ;


/* VIDT structure

 * Virtual interrupt vector:
 *	0xfffff000 : {handler, stack}[0x100]	- virtual interrupt vector

 * Contexts saved by kernel :
 *	0xfffff800 : context int
 *		0xfffff800 : registers
 *		0xfffff820 : eflags
 *		0xfffff824 : eip
 *		0xfffff828 : valid flag
 *	0xfffff840 : context isr
 *		// same
 *
 * Contexts saved by parent
 *		0xfffff900 : int
 *		0xfffff940 : isr

 * Contexts saved by self to cope w/ race conditions while resuming
 *		0xfffff980 : int
 *		0xfffff9C0 : isr

 * Virtual flags
 *  0xfffffffc : flags (VIF = 1)
 */
typedef struct __pack vidt_s
{
	struct __pack vidt_vint_s
	{
		uint32_t eip;
		uint32_t esp;
	} vint[0x100];
	/* always set by kernel */
/* 800h: */
	vidt_int_ctx_t intCtx;
	vidt_int_ctx_t isrCtx;
	vidt_int_ctx_t notifyChildCtx;
	vidt_int_ctx_t notifyParentCtx;
/* 900h:*/
	uint32_t rfu2[(0xffc-0x900)/4];
/* FFCh: */
	uint32_t flags;
} vidt_t;

#define CURRENT_VIDT ((vidt_t *)0xfffff000)

#define INTERRUPT_HANDLER(a, n)    extern void a(); void n(uint32_t data1, uint32_t data2, uint32_t caller) {
#define END_OF_INTERRUPT           resume(caller, 0);}

void Pip_RegisterInterrupt(uint32_t intno, void* handler, uint32_t* stack);
void Pip_VCLI(void); // Virtual IRQ disable
void Pip_VSTI(void); // Virtual IRQ enable
void Pip_InitVIDT(void*);

#endif
#endif
