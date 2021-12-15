#include "rt64.h"

static void startap(void)
{
	// _binary_out_*_{start,end,size} are linker defined symbols.
	extern u8 _binary_out_bootap_start[], _binary_out_bootap_size[];
	extern void entry32mp(void);

	// Write entry code to unused low memory at 0x7000.
	u8 *code = P2V(0x7000);
	memmove(code, _binary_out_bootap_start, (usize)_binary_out_bootap_size);

	for (struct percpu *cpu = cpus; cpu < cpus + ncpu; cpu++) {
		if (cpu == curcpu)
			continue;
		cprintf("startap: starting %d\n", cpu - cpus);

		// Arguments to AP boot
		// code-16	=> [int64] kernel stack for AP in 64-bit mode
		// code-8	=> [int32] physical
		// code-4	=> [int32] stack in use until 64-bit mode
		// code: 0x7000 => bootap.S code
		*(u32 *)(code - 4) = 0x8000;
		*(u32 *)(code - 8) = V2P(entry32mp);
		*(u64 *)(code - 16) = (u64)(kalloc() + KSTACK_SZ);

		// Send INIT and STARTUP IPIs to APs.
		// AP will start in real mode upon receiving startup IPI (SIPI).
		// That is `start` in bootap.S.
		lapicstartap(cpu->lapicid, V2P(code));

		// Wait for AP boot to finish
		while (cpu->started == 0)
			;
		cprintf("startap: started %d\n", cpu - cpus);
	}

	cprintf("startap: all ap started\n");
}

// BSP starts here
int main(void)
{
	uartearlyinit();
	kallocinit();
	paginginit_bsp();
	acpiinit();

	lapicinit();
	trapinit();
	procinit();
	uvminit();

	startap();
	while (1)
		;
}

void mpenter(void)
{
	paginginit_ap();
	trapinit();
	xchg64((isize *)&curcpu->started, 1); // tell startothers() we're up
	while (1)
		;
}
