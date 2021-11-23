#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	paginginit();

	u32 x = 0xfec00000;
	// testing cprintf
	cprintf("%d %l %l %d\n", 1000000000, 10000000000, 1000000000,
		10000000000);
	cprintf("%x %y %y %x\n", 0xdeadbeef, 0xcafebabedeadbeef, 0xdeadbeef,
		0xcafebabedeadbeef);
	cprintf("%p\n", main);
	cprintf("%p\n", 0);
	cprintf("Early uart test done\n", 0);

	cprintf("main: BSP starting rt64 ...\n");
	acpiinit();
	lapicinit();
	cprintf("main: BSP lapicid = %d\n", lapicid());

	while (1)
		;
}

void mpenter(void)
{
}
