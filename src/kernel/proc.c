#include "rt64.h"

#define KSTACK_SZ (PG_SZ4K * KSTACK_PAGES)

static u8 kstacks[MAX_PROC][KSTACK_SZ] ATTR_PAGEALIGN;
struct proc procs[MAX_PROC];
// For now: alloc proc just uses nproc++ -> no deletion of processes
int nproc;
__thread struct proc *curproc;

void sched(void)
{
	if (curproc->state == RUNNING)
		panic("sched running");
	if (readeflags() & FL_IF)
		panic("sched interruptible");

	struct proc *old = curproc;
#ifdef DEBUG_SCHED
	cprintf("proc: sched %s -> %s\n", curproc->name,
		curcpu->idleproc->name);
#endif
	curproc = curcpu->idleproc;
	lcr3(V2P(curproc->pt_root));
	swtch(&old->ctx, curcpu->idlectx);
	ASSERT(curproc != curcpu->idleproc);
}

void yield(void)
{
	ASSERT(curproc->state == RUNNING);
	cli(); // will sti in idle
	curproc->state = RUNNABLE; // must be after cli
	sched();
}

// Insert the current thread (kerninit i.e. idle) into the process queue
void procinit(void)
{
	// This is a form of dynamic allocation actually
	curproc = &procs[nproc++];
	curproc->state = RUNNING;
	safestrcpy(curproc->name, "idle", 16);
	// XXX: better be sprintf
	curproc->name[5] = '0' + curcpu->index;
	curproc->name[6] = '\0';
	// kstack field is used for returning from user.
	// the init idle task does not accept returning from user.
	// nor does it have dedicated kstacks[*] pages.
	curproc->kstack = NULL;
	curproc->pt_root = kpml4;

	curproc->pid = nproc;

	curcpu->idlectx = &curproc->ctx;
	curcpu->idleproc = curproc;

	cprintf("[%d] proc: init\n", curcpu->index);
}

struct proc *find_runnable_proc(void)
{
	static int i = 0; // round robin
	while (1) {
		if (procs[i].state == RUNNABLE) {
			int old = i;
			i = (i + 1) % nproc;
			return &procs[old];
		}
		i = (i + 1) % nproc;
	}
	return NULL;
}

void idlemain(void)
{
	for (;;) {
#ifdef DEBUG_SCHED
		cprintf("idle: enter\n");
#endif
		sti();

		// TODO: lock ptable
		struct proc *p = 0;
		if ((p = find_runnable_proc())) {
			curproc = p;
			curproc->state = RUNNING;
#ifdef DEBUG_SCHED
			cprintf("proc: sched %s -> %s\n",
				curcpu->idleproc->name, curproc->name);
#endif
			// task switch boilerplate
			lcr3(V2P(curproc->pt_root));
			set_task_kstack();

			swtch(curcpu->idlectx, &curproc->ctx);
			ASSERT(curproc == curcpu->idleproc);
		} else {
#ifndef CONFIG_IDLE_POOL
			hlt(); // WFI.
#endif
		}
	}
}

struct proc *spawn(const char *name, void *(*func)(void *), void *initarg)
{
	struct proc *p = &procs[nproc++];
	safestrcpy(p->name, name, 16);
	p->pid = nproc;

	memset(&p->ctx, 0, sizeof(struct context));
	p->ctx.rip = (u64)func;
	p->kstack = kstacks[p->pid];
	p->pt_root = kpml4;

	p->ctx.rsp = (u64)p->kstack + KSTACK_SZ;
	p->ctx.rsp -= XLENB;
	p->ctx.rbp = (u64)p->kstack + KSTACK_SZ;

	p->ctx.rdi = (u64)initarg;

	// Set RUNNABLE only when everything has been setup
	p->state = RUNNABLE;

	return p;
}

struct proc *spawnuser(const char *name, void *(*func)(void *), void *initarg)
{
	struct proc *p = &procs[nproc++];
	safestrcpy(p->name, name, 16);
	p->pid = nproc;

	memset(&p->ctx, 0, sizeof(struct context));
	p->ctx.rip = (u64)trapret;
	p->kstack = kstacks[p->pid];
	p->pt_root = upml4;
	p->ctx.rsp = (u64)p->kstack + KSTACK_SZ;
	p->ctx.rbp = (u64)p->kstack + KSTACK_SZ;

	p->ctx.rsp -= sizeof(struct trapframe);
	p->tf = (void *)p->ctx.rsp;
	memset(p->tf, 0, sizeof(struct trapframe));

	p->ctx.rsp -= XLENB; // save area for return pointer: used by swtch
	cprintf("spawnuser: kernel rsp = %p\n", p->ctx.rsp);

	void *ustack = kalloc();
	p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
	p->tf->rsp = (usize)ustack + PG_SZ4K - XLENB;
	p->tf->rflags = FL_IF;
	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
	p->tf->rip = (u64)func;
	p->tf->rdi = (u64)initarg;

	// Set RUNNABLE only when everything has been setup
	p->state = RUNNABLE;

	return p;
}

void exit(void)
{
	ASSERT(curproc->state == RUNNING);
	cli();
	curproc->state = EXITED;
	sched();
	panic("proc: exit unreachable");
}

void sleep(int nticks)
{
	if (nticks < 0)
		panic("cannot sleep %d ticks", nticks);
	if (nticks == 0)
		return;
	ASSERT(curproc->state == RUNNING);
	cli(); // will sti in idle
	curproc->state = SLEEPING; // must be after cli
	curproc->sleeprem = nticks;
	sched();
}
