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

	uvm_init(); // XXX: not here
	extern void *uhello(void *arg);
	extern void *minctestmain(void *arg);
	spawnuser("uhello", uhello, NULL);
	//spawn("master", minctestmain, NULL);

	idlemain();
}

void mpenter(void)
{
}
