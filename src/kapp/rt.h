#pragma once

#include "rt64.h"
#include "common.h"

#undef printf
#define printf cprintf
#define MINIMAL
#define USE_MUSL

struct timespec {
	u64 tsc;
};

static inline void tsnorm(struct timespec *ts)
{
	// pass
}

static inline void tsinc(struct timespec *dst, const struct timespec *delta)
{
	ASSERT(dst->tsc + delta->tsc > dst->tsc);
	dst->tsc += delta->tsc;
}

static inline void tssetus(struct timespec *dst, int us)
{
	dst->tsc = us * TSC_FREQ / USEC_PER_SEC;
}

static inline long tsdelta(const struct timespec *t1, const struct timespec *t2)
{
	long d = t1->tsc - t2->tsc;
	if (d <= 0) {
		extern u64 old_now;
		extern u64 slnext;
		extern int sleep_ticks;
		panic("invalid tsdelta %l = %l - %l\n"
		      "old_now = %l, slnext = %l, sleep_ticks = %d\n",
		      d, t2->tsc, t1->tsc, old_now, slnext, sleep_ticks);
	}
	d = (d + 2600 - 1) / 2600; // ceiling div
	ASSERT(d != 0);
	return d;
}

static inline int tsgreater(struct timespec *a, struct timespec *b)
{
	return a->tsc > b->tsc;
}

static inline void tsgettime(struct timespec *d)
{
	d->tsc = rdtsc();
}

static inline void tssleepto(struct timespec *d)
{
	// sleep this many TSC
	u64 rem = d->tsc - rdtsc();
	// sleep this many lapic timer counts
	rem = TSC_TO_LAPIC(rem);
	// one shot: how many remaining;
	rem -= lapictimercnt();
	// ceil to how many ticks to sleep
	rem = (rem + LAPIC_TIMER_PERIOD - 1) / LAPIC_TIMER_PERIOD;
	//DEBUG cprintf("sleeping: %d\n", rem);
	rem += 2;
	extern int sleep_ticks;
	sleep_ticks = rem;
	// cprintf("@ %d\n", sleep_ticks);
	sleep(rem);
}

static inline void statdelay(void)
{
	sleep(100);
}

struct thread_t {
	struct proc *v;
};

static inline void spawnthr(thread_t *thr, void *(*func)(void *), void *arg)
{
	thr->v = spawn("timerthr", func, arg);
}

// unimplemented for kernel
static inline void setaff(int cpu)
{
	// pass
}
