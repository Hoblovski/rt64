#include "rt64.h"

//static u64 kpml4[PG_NENT] ATTR_PAGEALIGN;
//static u64 kpdpt[PG_NENT] ATTR_PAGEALIGN;
//static u64 iopd[PG_NENT] ATTR_PAGEALIGN;
// TODO: should I strip dynamic allocation?

u64 *kpml4 = NULL;

static void safe_setent(u64 *pt, usize idx, u64 ent)
{
	ASSERT(!(pt[idx] & PF_P));
	pt[idx] = ent;
}

static u64 *safe_nextpt_mayalloc(u64 *pt, usize idx)
{
	u64 ent = pt[idx];
	if (ent & PF_P) {
		usize pa = PE_ADDR(ent);
		return P2V(pa);
	}
	u64 *nextpt = kalloc();
	safe_setent(pt, idx, V2P(nextpt) | PF_WP);
	return nextpt;
}

/*
 * flags: used in combination with PF_P and PF_PS
 */
static void paging_map_1G(u64 *pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ1G));
	ASSERT(ALIGNED(pa, PG_SZ1G));

	u64 *pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	u64 ent = pa | flags | PF_P | PF_PS;
	safe_setent(pdpt, PDPT_IDX(va), ent);
}

/*
 * flags: used in combination with PF_P and PF_PS
 */
static void paging_map_2M(u64 *pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ2M));
	ASSERT(ALIGNED(pa, PG_SZ2M));

	u64 *pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	u64 *pd = safe_nextpt_mayalloc(pdpt, PDPT_IDX(va));
	u64 ent = pa | flags | PF_P | PF_PS;
	safe_setent(pd, PD_IDX(va), ent);
}

static void paging_map_4K(u64 *pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ4K));
	ASSERT(ALIGNED(pa, PG_SZ4K));

	u64 *pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	u64 *pd = safe_nextpt_mayalloc(pdpt, PDPT_IDX(va));
	u64 *pt = safe_nextpt_mayalloc(pd, PT_IDX(va));
	u64 ent = pa | flags | PF_P;
	safe_setent(pt, PT_IDX(va), ent);
}

static void paging_map(u64 *pml4, usize va, usize pa, usize sz, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ4K));
	ASSERT(ALIGNED(pa, PG_SZ4K));
	ASSERT(ALIGNED(sz, PG_SZ4K));

	while (sz > 0) {
		if (sz >= PG_SZ1G && ALIGNED(va | pa, PG_SZ1G)) {
			paging_map_1G(pml4, va, pa, flags);
			va += PG_SZ1G;
			pa += PG_SZ1G;
			sz -= PG_SZ1G;
			continue;
		}
		if (sz >= PG_SZ2M && ALIGNED(va | pa, PG_SZ2M)) {
			paging_map_2M(pml4, va, pa, flags);
			va += PG_SZ2M;
			pa += PG_SZ2M;
			sz -= PG_SZ2M;
			continue;
		}
		if (sz >= PG_SZ4K && ALIGNED(va | pa, PG_SZ4K)) {
			paging_map_4K(pml4, va, pa, flags);
			va += PG_SZ4K;
			pa += PG_SZ4K;
			sz -= PG_SZ4K;
			continue;
		}
	}
}

// Switch from bootloader's entrypml4 to kernel's pml4.
void paginginit_bsp(void)
{
	kpml4 = kalloc();

	// 0xFFFF_FFFF_8000_0000 ... 0xFFFF_FFFF_FFFF_FFFF (+2G) -> 0x0 (+2G)
	// 0xFFFF_FFFF_4000_0000 ... (+32M) -> 0xFE00_0000 ... 0xFFFF_FFFF (+32M)
	paging_map(kpml4, KERNBASE, 0, 0x80000000, PF_W);
	paging_map(kpml4, DEVBASE, 0xFE000000, 16 * PG_SZ2M, PF_PWT | PF_PCD);

	lcr3(V2P(kpml4));
	cprintf("paing: bsp init\n");
}

void paginginit_ap(void)
{
	ASSERT(kpml4);
	lcr3(V2P(kpml4));
	cprintf("paing: ap init\n");
}
