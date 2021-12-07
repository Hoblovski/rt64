#include "rt64.h"

static u64 min, max, sum, act, cycles;
static int prev_sleep_ticks, prev_sleep_tsc;

static inline void sleepto(u64 tsc)
{
	u64 remtsc = tsc - rdtsc();
	prev_sleep_tsc = remtsc;
	u64 remlapic = TSC_TO_LAPIC(remtsc);
	remlapic -= lapictimercnt();
	u64 remtick = (remlapic + LAPIC_TIMER_PERIOD - 1) / LAPIC_TIMER_PERIOD;
	prev_sleep_ticks = remtick;
	sleep(remtick);
}

#define TSC_INTERVAL 52000000 // 20ms
static void *timerthread(void *_arg)
{
	min = 1000000;
	max = 0;
	sum = 0;
	cycles = 0;

	u64 now = rdtsc();
	u64 next = now + TSC_INTERVAL;
	while (1) {
		// sleepto(next);
		now = rdtsc();

		if (now <= next)
			panic("now=%l, next=%l, diff=%l\n"
			      "prev_sleep_ticks=%l\n"
			      "prev_sleep_tsc=%l\n",
			      now, next, now - next, prev_sleep_ticks,
			      prev_sleep_tsc);

		u64 diff = now - next;
		if (diff < min)
			min = diff;
		if (diff > max)
			max = diff;
		sum += diff;
		act = diff;
		cycles++;

		while (next <= now)
			next += TSC_INTERVAL;
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
}

int minctestmain()
{
	ASSERT(spawn("timerthread", timerthread, NULL) != NULL);

	while (cycles == 0)
		sleep(10);

	while (1) {
		char *fmt = "C:%7l Min:%7l Act:%5l Avg:%5l Max:%8l";
		cprintf(fmt, cycles, min, act, sum / cycles, max);

		sleep(10);
	}
}
