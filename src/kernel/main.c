#include "rt64.h"

static void *kthread1(void *arg)
{
	for (int j = 0; j < 2; j++) {
		cprintf("  kthread1: got me\n");
		yield();
	}
	exit();
}

static void *kthread2(void *arg)
{
	for (int j = 0; j < 2; j++) {
		cprintf("  kthread2: got me\n");
		for (volatile int i = 0; i < 100000000; i++)
			;
	}
	exit();
}

// BSP starts here
int main(void)
{
	uartearlyinit();
	paginginit_bsp();
	acpiinit();
	lapicinit();
	trapinit();
	procinit();
	spawn("kthread1", kthread1);
	spawn("kthread2", kthread2);

	idlemain();
}

void mpenter(void)
{
}
