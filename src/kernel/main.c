#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	paginginit_bsp();
	acpiinit();
	lapicinit();
	trapinit();
	sti();

	while (1)
		;
}

void mpenter(void)
{
}
