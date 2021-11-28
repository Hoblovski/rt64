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
	cprintf("proc: sched %s -> %s\n", curproc->name,
		curcpu->idleproc->name);

	curproc = curcpu->idleproc;
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
		cprintf("idle: enter\n");
		sti();

		// TODO: lock ptable
		struct proc *p = 0;
		if ((p = find_runnable_proc())) {
			curproc = p;
			curproc->state = RUNNING;
			//			cprintf("proc: sched %s -> %s\n",
			//				curcpu->idleproc->name, curproc->name);
			swtch(curcpu->idlectx, &curproc->ctx);
			ASSERT(curproc == curcpu->idleproc);
		} else {
			hlt(); // WFI. could harm real-time?
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

	p->ctx.rsp = (u64)p->kstack + KSTACK_SZ;
	p->ctx.rbp = (u64)p->kstack + KSTACK_SZ;

	p->ctx.rdi = (u64)initarg;

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
}

void sleep(int nticks)
{
	ASSERT(curproc->state == RUNNING);
	cli(); // will sti in idle
	curproc->state = SLEEPING; // must be after cli
	curproc->sleeprem = nticks;
	sched();
}
