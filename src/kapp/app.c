#include "rt64.h"

static void *kthread1(void *arg)
{
	cprintf("  kthread1: initarg=%d\n", (u64)arg);
	for (int j = 0; j < 2; j++) {
		cprintf("  kthread1: got me %d\n", (u64)arg);
		yield();
	}
	exit();
}

static void *kthread2(void *arg)
{
	cprintf("  kthread2: initarg=%d\n", (u64)arg);
	for (int j = 0; j < 2; j++) {
		cprintf("  kthread2: got me %d\n", (u64)arg);
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
