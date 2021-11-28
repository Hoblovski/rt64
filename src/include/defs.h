#pragma once

// ======== kernel.ld
extern char end[];

// ======== vectors.S
extern void *vectors[];

// ======== entry.S
void wrmsr(u32 msr, u64 val);

// ======== uart.c
/*
 * Init uart with polling.
 */
void uartearlyinit(void);
void microdelay(int us);
void uartputc(int c);

// ======== console.c
void cprintf(char *fmt, ...);
void panic(char *fmt, ...) __attribute__((noreturn));

// ======== acpi.c
void acpiinit(void);
extern struct percpu cpus[MAX_CPU];
extern int ncpu;
extern u8 ioapicid;

// ======== lapic.c
void lapicinit(void);
u32 lapicid(void);
void lapiceoi(void);
/*
 * In one shot mode, it is a count-down register, starting from initial count.
 * Otherwise it can be used to serve a similar purpose as TSC.
 */
u32 lapictimercnt(void);

// ======== paging.c
void paginginit_bsp(void);
void paginginit_ap(void);

// ======== trap.c
void trapinit();
extern __thread struct percpu *curcpu;
extern int ticks;

// ======== switch.S
/*
 * Before swtch, must update curproc.
 */
void swtch(struct context *old, struct context *new);

// ======== proc.c
extern struct proc procs[MAX_PROC];
extern __thread struct proc *curproc;
extern int nproc;
void idlemain(void) __attribute__((noreturn));
void procinit(void);
/*
 * Spawn a function.
 * NOTE: Either the function should always end by an `exit`, or it should be __attribute__((noreturn)).
 */
struct proc *spawn(const char *name, void *(*func)(void *), void *initarg);
/*
 * Volutarily give up execution for current thread, but remain runnable.
 */
void yield(void);
/*
 * Terminate execution of current thread.
 */
void exit(void) __attribute__((noreturn));
/*
 * Sleep until the specified number of ticks has elapsed
 * Real sleep time should be	TICK_INTERVAL * [nticks-1, nticks + eps]
 *
 * Should we use TSC for better resolution?
 */
void sleep(int nticks);
