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

static void *kthread3(void *arg)
{
	extern u64 test_tsc;
	u64 tsc;

	for (int i = 0; i < 50; i++) {
		tsc = rdtsc();
		cprintf("  kthread3: ticks=%d, tscd=%l, going to sleep\n",
			ticks, tsc - test_tsc);
		sleep(10);
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
	//spawn("kthread1", kthread1);
	//spawn("kthread2", kthread2);
	spawn("sleep", kthread3);

	idlemain();
}

void mpenter(void)
{
}
