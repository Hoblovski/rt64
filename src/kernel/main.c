#include "rt64.h"

char a[4];
// BSP starts here
int main(void)
{
	uartearlyinit();
	cprintf("BSP starting rt64\n\n");
	cprintf("BSP starting rt64\n\n");
	cprintf("%d %p %p %x\n", 124, a + 1, a + 2, 0xdeadbeef);
	panic("me panic");
	cprintf("BSP starting rt64\n\n");

	while (1)
		;
}

void mpenter(void)
{
}
