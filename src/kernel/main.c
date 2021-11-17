#include "rt64.h"

char a[4];
// BSP starts here
int main(void)
{
	uartearlyinit();
	u32 x = 0xfec00000;
	cprintf("main: BSP starting rt64 %x ...\n", x);
	acpiinit();

	while (1)
		;
}

void mpenter(void)
{
}
