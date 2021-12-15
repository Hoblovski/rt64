#include "rt64.h"

static u64 min, max, sum, act, cycles;

static void updatestats(u64 diff)
{
	if (diff < min)
		min = diff;
	if (diff > max)
		max = diff;
	sum += diff;
	act = diff;
	cycles++;
}

#define LAPIC_SLEEP_TICKS 1
static void *timerthread(void *_arg)
{
	min = 1000000;
	max = 0;
	sum = 0;
	cycles = 0;
	sleep(1);

	u64 lastticks = ticks;

	while (1) {
		sleep(LAPIC_SLEEP_TICKS);
		u64 nowcct = lapictimercnt();
		u64 nowticks = ticks;
		ASSERT(nowticks == lastticks + LAPIC_SLEEP_TICKS);
		lastticks = nowticks;
		u64 nowrem = LAPIC_TIMER_PERIOD - nowcct;

		updatestats(nowrem);
	}
}

void measure_sleep_tsc(void)
{
#define N 15
	static int l = 0;
	static int a[N];
	sleep(1);
	while (1) {
		a[l++] = rdtsc();
		sleep(1);
		if (l == N) {
			cprintf("@> ");
			for (int i = 0; i < N - 1; i++)
				cprintf("%l  ", a[i + 1] - a[i]);
			sleep(1);
			l = 0;
		}
	}
#undef N
}

int minctestmain()
{
	struct proc *p = spawn(&(struct newprocdesc){ .name = "timer",
						      .func = timerthread,
						      .initarg = NULL,
						      .prio = 0 });
	ASSERT(p != NULL);

	while (cycles == 0)
		sleep(10);

	while (1) {
		char *fmt = "C:%7l Min:%7l Act:%5l Avg:%5l Max:%8l\n";
		cprintf(fmt, cycles, min, act, sum / cycles, max);

		sleep(10);
	}
}
