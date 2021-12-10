#include "rt64.h"

__thread struct percpu *curcpu;

// x64 IDT has 256 entries, each of 16B (sdm3 fig 6-8)
// XXX: make more reasonable
static u32 idt[PG_SZ4K / 4] ATTR_PAGEALIGN;

static void mkgate(u32 n, void *kva, u32 pl, u32 trap)
{
	u64 addr = (u64)kva;
	n *= 4;
	trap = trap ? 0x8F00 : 0x8E00; // TRAP vs INTERRUPT gate;
	idt[n + 0] = (addr & 0xFFFF) | ((SEG_KCODE << 3) << 16);
	idt[n + 1] =
		(addr & 0xFFFF0000) | trap | ((pl & 3) << 13); // P=1 DPL=pl
	idt[n + 2] = addr >> 32;
	idt[n + 3] = 0;
}

void set_task_kstack(void)
{
	ASSERT(curproc !=
	       curcpu->idleproc); // idleproc->kstack do not mean anything

	u32 *tss = (void *)curcpu->tls + 1024;
	usize rsp = (usize)curproc->kstack + KSTACK_SZ;
	tss[1] = rsp;
	tss[2] = rsp >> 32;
}

// TLS holds GDT and TSS and __thread data. One page for each cpu.
static u8 tls[MAX_CPU][PG_SZ4K] ATTR_PAGEALIGN;

__thread struct cpu *cpu;

// TODO: split to percpu-init and primary-init
// Setup IDT, TLS, GDT and TSS.
void trapinit(void)
{
	u8 lapic_id = lapicid();
	int cpuid = -1;
	for (int i = 0; i < ncpu; i++)
		if (lapic_id == cpus[i].index) {
			cpuid = i;
			break;
		}
	ASSERT(cpuid != -1);

	// setup idt
	for (int n = 0; n < 256; n++)
		mkgate(n, vectors[n], 0, 0);
	mkgate(T_SYSCALL, vectors[T_SYSCALL], 3, 1); // T_SYSCALL
	lidt((void *)idt, PG_SZ4K);

	// TLS page
	void *tls_local = &tls[cpuid];
	// Allocate 1024 bytes (256 entries) for GDT
	u64 *gdt = (u64 *)tls_local;
	// TSS is 1024 Bytes
	u32 *tss = (u32 *)(((char *)tls_local) + 1024);
	// The remaining of TLS page: used for __thread (via fsbase)
	wrmsr(0xC0000100, ((u64)tls_local) + 2048);

	curcpu = &cpus[cpuid]; // Must be after initializing FSBASE
	curcpu->tls = tls_local;
	ASSERT(curcpu == &cpus[cpuid]);

	// Setup TSS
	// WTF: sdm
	tss[16] = 0x00680000; // IO Map Base = End of TSS

	// Setup GDT
	gdt[0] = 0x0000000000000000;
	gdt[SEG_KCODE] = 0x0020980000000000; // Code, DPL=0, R/X
	gdt[SEG_UCODE] = 0x0020F80000000000; // Code, DPL=3, R/X
	gdt[SEG_KDATA] = 0x0000920000000000; // Data, DPL=0, W
	gdt[SEG_KCPU] = 0x0000000000000000; // unused
	gdt[SEG_UDATA] = 0x0000F20000000000; // Data, DPL=3, W

	// WTF: sdm
	u64 addr = (u64)tss;
	gdt[SEG_TSS + 0] = (0x0067) | ((addr & 0xFFFFFF) << 16) |
			   (0x00E9LL << 40) | (((addr >> 24) & 0xFF) << 56);
	gdt[SEG_TSS + 1] = (addr >> 32);

	lgdt((void *)gdt, 8 * sizeof(u64));

	ltr(SEG_TSS << 3);

	cprintf("trap: init\n");
}

u64 test_tsc;
int ticks;
void trap(struct trapframe *tf)
{
	test_tsc = rdtsc();

	switch (tf->trapno) {
	case T_IRQ0 + IRQ_TIMER:
		ticks++;
#ifdef DEBUG_TICK
		{
			u64 rsp, tsc;
			tsc = rdtsc();
			asm volatile("movq %%rsp, %0" : "=r"(rsp));
			static u64 last_tsc;
			if (ticks % CONFIG_TICK_INTERVAL == 0)
				cprintf("timer: ticks=%d, rsp=%p, tsc=%l, tscint=%l, curproc=%s\n",
					ticks, rsp, tsc, tsc - last_tsc,
					curproc->name);
			last_tsc = tsc;
		}
#endif

		for (int i = 0; i < nproc; i++)
			if (procs[i].state == SLEEPING)
				if (--procs[i].sleeprem == 0) {
					procs[i].state = RUNNABLE;
				}
		lapiceoi();
		break;
	case T_SYSCALL:
		tf->rax = syscall(tf->rax, tf->rdi, tf->rsi, tf->rdx, tf->rcx,
				  tf->r8, tf->r9);
		break;
	default:
		panic("unexpected trap %d from cpu %d rip %p (cr2=0x%x)\n",
		      tf->trapno, curcpu->index, tf->rip, rcr2());
	}
}

isize syscall(isize num, usize a0, usize a1, usize a2, usize a3, usize a4,
	      usize a5)
{
	switch (num) {
	case SYS_print:
		return sys_print((const char *)a0);
	default:
		return -1;
	}
}

isize sys_print(const char *a0)
{
	cprintf("%s", a0);
	return 0;
}
