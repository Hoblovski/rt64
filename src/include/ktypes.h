#pragma once

struct percpu;
struct context;
struct trapframe;
struct proc;

// Per-CPU state
struct percpu {
	// cpuid: normalized (i.e. successive numbers from 0) based on lapicid
	// INVARIANT: forall i, cpus[i].index = i
	u8 index;
	// Local APIC ID
	u8 lapicid;

	// Context of the IDLE kernel thread
	struct context *idlectx;
	struct proc *idleproc;

	// TLS storage, holds GDT and TSS
	void *tls;
};

enum procstate {
	// So by default zero PCB is UNINIT PCB
	UNINIT = 0,
	RUNNABLE,
	RUNNING,
	EXITED,
	SLEEPING
};

struct proc {
	enum procstate state;
	char name[16];
	// INVARIANT: forall i, proc[i].pid == i
	volatile int pid;
	// lowest byte of kernel stack
	void *kstack;
	// PML4.
	// For user threads, it will be `upml4`.
	//	Ideally every threads will have its own pml4.
	// For kernel threads, it will be `kpml4`.
	u64 *pt_root;

	// Remaining ticks to sleep, only valid if self.state == SLEEPING
	int sleeprem;

	// This is actually used for return to user...
	// So when it's kernel only, we do not need it?
	struct trapframe *tf;

	// state save area of kernel context switching
	struct context ctx;
};
