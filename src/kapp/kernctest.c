/*
 * Build with

gcc cyclictest.c -lpthread

 * or

musl-gcc cyclictest.c -lpthread -DUSE_MUSL

 * Terminate with
<Ctrl-\>	i.e. SIGKILL

 *
 * NOTE THAT FOR ACCURATE RESULTS    /dev/cpu_dma_latency    NEEDS TO BE SET TO 0.
 * See Documentation/power/pm_qos_interface.txt .
 *
 * DEFINES: USE_MUSL    MINIMAL
 */
#include "rt.h"

struct thread_param {
	int id;
	thread_t thread;
	unsigned long interval;
	int prio;
	int policy;
	int cpu; // which cpu to run on
};

struct thread_stat {
	int tid;
	long max;
	long min;
	long act;
	long sum; // not using `double avg`
	int cycles;
};

static int ginterval = DEFAULT_INTERVAL;
static int priority = 0;
static struct thread_param thrpar[NUM_THREADS];
static struct thread_stat thrstat[NUM_THREADS];
static int shutdown = 0;

u64 old_now;
u64 slnext;
int sleep_ticks;

static void *timerthread(void *param)
{
	int err;

	// init
	struct thread_param *par = param;
	struct thread_stat *stat = &thrstat[par->id];
#ifdef USE_MUSL
	// musl does not have gettid
	stat->tid = 0;
#else
	stat->tid = gettid();
#endif

	// WHY BLOCK SIGALRM
	// XXX

	// priority
#ifndef USE_MUSL
	struct sched_param schedp;
	memset(&schedp, 0, sizeof(schedp));
	schedp.sched_priority = par->prio; // 0
	err = sched_setscheduler(0, par->policy, &schedp);
	assert(!err);
#endif

	// affinity
	setaff(par->cpu);

	// INIT TIMER
	struct timespec now, next, interval;
	tssetus(&interval, ginterval);

	tsgettime(&now);
	next = now;
	tsinc(&next, &interval);
	while (!shutdown) {
		old_now = now.tsc;
		slnext = next.tsc;
		tssleepto(&next);
		tsgettime(&now);
		long diff = tsdelta(&now, &next);
		ASSERT(diff > 0);

		if (diff < stat->min)
			stat->min = diff;
		if (diff > stat->max)
			stat->max = diff;
		stat->act = diff;
		stat->sum += diff;
		stat->cycles++;

		tsinc(&next, &interval);
		while (tsgreater(&now, &next))
			tsinc(&next, &interval);
	}

	while (1)
		;
}

__attribute__((unused)) static void sighand(int sig)
{
	shutdown = 1;
}

static void print_stat(struct thread_param *par, struct thread_stat *stat)
{
	int index = par->id;

	char *fmt = "T:%2d (%5d) P:%2d I:%ld C:%7l "
		    "Min:%7l Act:%5l Avg:%5l Max:%8l";

	printf(fmt, index, stat->tid, par->prio, DEFAULT_INTERVAL, stat->cycles,
	       stat->min, stat->act,
	       stat->cycles ? (long)(stat->sum / stat->cycles) : 0, stat->max);
	//printf("\r"); // reuse the same line, DEBUG
	printf("\n"); // reuse the same line
}

void *ctestmain(void *arg)
{
	int err;

#ifndef MINIMAL
	signal(SIGINT, sighand);
#endif

	for (int i = 0; i < NUM_THREADS; i++) {
		struct thread_param *par = &thrpar[i];
		struct thread_stat *stat = &thrstat[i];
		par->id = i;
		par->cpu = i % MAX_CPUS;
		par->prio = priority;
#ifndef USE_MUSL
		par->policy = SCHED_OTHER;
#endif

		stat->min = 10000;
		stat->max = 0;
		stat->sum = 0;
		stat->cycles = 0;

		spawnthr(&par->thread, timerthread, par);
	}

	while (!shutdown) {
		for (int i = 0; i < NUM_THREADS; i++) {
			if (thrstat[i].cycles > 210)
				continue;
			print_stat(&thrpar[i], &thrstat[i]);
		}

		if (shutdown)
			break;

		statdelay();
	}

	while (1)
		;
}
