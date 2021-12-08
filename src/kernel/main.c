#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	kallocinit();
	paginginit_bsp();
	acpiinit();
	lapicinit();
	trapinit();
	procinit();
	extern void *minctestmain(void *arg);
	spawn("masterthread", minctestmain, NULL);

	extern void *uhello(void *arg);
	cprintf("uhello is %p\n", uhello);
	extern char ubegin[], uend[];
	cprintf("user is %p %p\n", ubegin, uend);
	cprintf("syscall from kernel: %d\n", uhello("yes"));

	idlemain();
}

void mpenter(void)
{
}
