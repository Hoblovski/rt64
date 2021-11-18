#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	paginginit();

	u32 x = 0xfec00000;
	cprintf("main: BSP starting rt64 %x ...\n", x);
	acpiinit();
	cprintf("main: BSP starting rt64 %x ...\n", x);

	while (1)
		;
}

void mpenter(void)
{
}
