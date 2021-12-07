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
	extern void *minctestmain(void *arg);
	spawn("masterthread", minctestmain, NULL);

	idlemain();
}

void mpenter(void)
{
}
