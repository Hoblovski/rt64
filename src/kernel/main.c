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
	cprintf("syscall from kernel: %d\n", uhello("yes"));

	idlemain();
}

void mpenter(void)
{
}
