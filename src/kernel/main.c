#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	paginginit_bsp();
	acpiinit();
	lapicinit();
	trapinit();
	procinit();
	extern void *ctestmain(void *arg);
	spawn("master", ctestmain, NULL);

	idlemain();
}

void mpenter(void)
{
}
