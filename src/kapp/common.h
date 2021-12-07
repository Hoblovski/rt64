#pragma once

/*
 * number of timerthreads
 */
#define NUM_THREADS 1
#define MAX_CPUS 12
#define DEFAULT_INTERVAL 1000 // in usecs
#define USEC_PER_SEC 1000000
#define NSEC_PER_SEC 1000000000

// will be overriten
#define printf

struct timespec;
static inline void tsnorm(struct timespec *ts);
static inline void tsinc(struct timespec *dst, const struct timespec *delta);
static inline void tssetus(struct timespec *dst, int us);

// return the difference in USECS
static inline long tsdelta(const struct timespec *t1,
			   const struct timespec *t2);
static inline int tsgreater(struct timespec *a, struct timespec *b);
static inline void tsgettime(struct timespec *d);
static inline void tssleepto(struct timespec *d);

static inline void statdelay(void);

typedef struct thread_t thread_t;
static inline void spawnthr(thread_t *thr, void *(*func)(void *), void *arg);

// unimplemented for kernel
static inline void setaff(int cpu);
