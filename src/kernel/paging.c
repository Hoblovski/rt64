#include "rt64.h"

static u64 kpml4[PG_NENT] __attribute__((__aligned__(PG_SZ4K)));
static u64 kpdpt[PG_NENT] __attribute__((__aligned__(PG_SZ4K)));
static u64 iopd[PG_NENT] __attribute__((__aligned__(PG_SZ4K)));

// Switch from bootloader's entrypml4 to kernel's pml4.
void paginginit(void)
{
	kpml4[511] = V2P(kpdpt) | PF_P | PF_W;

	// 0xFFFF_FFFF_8000_0000 ... 0xFFFF_FFFF_FFFF_FFFF (+2G) -> 0x0 (+2G)
	// Kernel virtual mapping.
	kpdpt[510] = 0x0 | PF_WP | PF_PS;
	kpdpt[511] = PG_SZ1G | PF_WP | PF_PS;

	// 0xFFFF_FFFF_4000_0000 ... (+32M) -> 0xFE00_0000 ... 0xFFFF_FFFF (+32M)
	kpdpt[509] = V2P(iopd) | PF_P | PF_W;
	usize paddr = DEVSPACE;
	for (int i = 0; i < 16; i++, paddr += PG_SZ2M)
		iopd[i] = paddr | PF_WP | PF_PS | PF_PWT | PF_PCD;

	lcr3(V2P(kpml4));
	cprintf("paing: paginginit done\n");
}
