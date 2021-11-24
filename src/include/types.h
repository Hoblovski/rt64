#pragma once

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;
typedef long isize;
typedef unsigned long usize;

// TODO: this is confusing
#ifdef STATIC_ASSERT
STATIC_ASSERT(sizeof(i16) == 2, bad_platform);
STATIC_ASSERT(sizeof(u16) == 2, bad_platform);
STATIC_ASSERT(sizeof(i32) == 4, bad_platform);
STATIC_ASSERT(sizeof(u32) == 4, bad_platform);
STATIC_ASSERT(sizeof(i64) == 8, bad_platform);
STATIC_ASSERT(sizeof(u64) == 8, bad_platform);
STATIC_ASSERT(sizeof(isize) == XLENB, bad_platform);
STATIC_ASSERT(sizeof(usize) == XLENB, bad_platform);
#endif

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

// Saved registers for kernel context switches.
//	Specifically when `switch` function is called. (active switch)
//
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
//
// Don't need to save %rax,
// because it will contain the return value (and thus caller-saved).
//
// Don't need to save %r10, %r11, %rdi, %rsi, %rdx, %rcx, %r8, %r9,
// because they are caller-saved by x64 callingconv.
//
// Contexts are stored at proc->context, and might be pointed to by percpu->idlectx.
//
// When calling `swtch`, %rip will be on top of the stack.
// But we still need this field, so we can designate a specific place to return to.
struct context {
	u64 r15; // 0
	u64 r14; // 8
	u64 r13; // 16
	u64 r12; // 24
	u64 rbx; // 32
	u64 rbp; // 40
	u64 rip; // 48
	u64 rsp; // 56
};

// Saved registers for passive context switches i.e. traps.
struct trapframe {
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rbp;
	u64 rsi;
	u64 rdi;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;

	// below is saved by hardware and vector*
	u64 trapno;
	u64 err;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
};

enum procstate {
	// By default zero PCB is UNINIT PCB
	UNINIT = 0,
	RUNNABLE,
	RUNNING
};

struct proc {
	enum procstate state;
	char name[16];
	// INVARIANT: forall i, proc[i].pid == i
	volatile int pid;

	// This is actually used for return to child...
	// So when it's kernel only, we do not need it?
	struct trapframe *tf;
	struct context ctx;
	void *kstack; // lowest byte of kernel stack
};
