/*
 * Page mapping etc.
 *
 * For rt64 we do not really have virtual memory,
 * all mappings (kernel & user) are offset based.
 * We use them only for protection i.e. kernel pages are not user accessible i.e. PF_U.
 *
 */
#include "rt64.h"

pt_t kpml4 = NULL;
pt_t upml4 = NULL;

static void safe_setent(pt_t pt, usize idx, u64 ent)
{
	int x = (!(pt[idx] & PF_P));
	ASSERT(x);
	pt[idx] = ent;
}

static void safe_updent(pt_t pt, usize idx, u64 ent)
{
	ASSERT(pt[idx] & PF_P);
	pt[idx] = ent;
}

static pt_t safe_nextpt_mayalloc(pt_t pt, usize idx)
{
	u64 ent = pt[idx];
	if (ent & PF_P) {
		ASSERT(!(ent & PF_PS)); // cannot nextpt on huge pages
		usize pa = PE_ADDR(ent);
		return P2V(pa);
	}
	pt_t nextpt = kalloc();
	safe_setent(pt, idx, V2P(nextpt) | PF_WP | PF_U);
	return nextpt;
}

static pt_t safe_nextpt_noalloc(pt_t pt, usize idx)
{
	u64 ent = pt[idx];
	ASSERT(ent & PF_P);
	ASSERT(!(ent & PF_PS)); // cannot nextpt on huge pages
	usize pa = PE_ADDR(ent);
	return P2V(pa);
}

static void paging_split(pt_t pt, usize idx, usize pgsz)
{
	ASSERT(pt[idx] & PF_P);
	ASSERT(pt[idx] & PF_PS);

	usize pa = PE_ADDR(pt[idx]), nextpgsz = pgsz / PG_NENT;
	u64 flags = PE_FLAGS(pt[idx]);
	if (nextpgsz == PG_SZ4K)
		flags &= ~PF_PS;

	pt_t nextpt = kalloc();
	for (int i = 0; i < PG_NENT; i++, pa += nextpgsz) {
		u64 ent = pa | flags;
		safe_setent(nextpt, i, ent);
	}
	u64 ent = (V2P(nextpt) | flags) & (~PF_PS);
	safe_setent(pt, idx, ent);
}

static pt_t safe_nextpt_split(pt_t pt, usize idx, usize pgsz)
{
	u64 ent = pt[idx];
	ASSERT(ent & PF_P);
	ASSERT(ent & PF_PS);
	paging_split(pt, idx, pgsz);
	ent = pt[idx];
	usize pa = PE_ADDR(ent);
	return P2V(pa);
}

/*
 * flags: used in combination with PF_P and PF_PS
 */
static void paging_map_1G(pt_t pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ1G));
	ASSERT(ALIGNED(pa, PG_SZ1G));

	pt_t pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	u64 ent = pa | flags | PF_P | PF_PS;
	safe_setent(pdpt, PDPT_IDX(va), ent);
}

/*
 * flags: used in combination with PF_P and PF_PS
 */
static void paging_map_2M(pt_t pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ2M));
	ASSERT(ALIGNED(pa, PG_SZ2M));

	pt_t pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	pt_t pd = safe_nextpt_mayalloc(pdpt, PDPT_IDX(va));
	u64 ent = pa | flags | PF_P | PF_PS;
	safe_setent(pd, PD_IDX(va), ent);
}

static void paging_map_4K(pt_t pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ4K));
	ASSERT(ALIGNED(pa, PG_SZ4K));

	pt_t pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	pt_t pd = safe_nextpt_mayalloc(pdpt, PDPT_IDX(va));
	pt_t pt = safe_nextpt_mayalloc(pd, PD_IDX(va));
	u64 ent = pa | flags | PF_P;

	//safe_setent(pt, PT_IDX(va), ent);
	//XXX
	pt[PT_IDX(va)] = ent;
}

static u64 *paging_getpteref(pt_t pml4, usize va)
{
	pt_t pt;
	u64 *ent;

	pt = pml4;
	ent = &(pt[PML4_IDX(va)]);
	if (!(*ent & PF_P))
		return NULL;
	if (*ent & PF_PS)
		return ent;

	pt = P2V(PE_ADDR(*ent));
	ent = &(pt[PDPT_IDX(va)]);
	if (!(*ent & PF_P))
		return NULL;
	if (*ent & PF_PS)
		return ent;

	pt = P2V(PE_ADDR(*ent));
	ent = &(pt[PD_IDX(va)]);
	if (!(*ent & PF_P))
		return NULL;
	if (*ent & PF_PS)
		return ent;

	pt = P2V(PE_ADDR(*ent));
	ent = &(pt[PT_IDX(va)]);
	if (!(*ent & PF_P))
		return NULL;
	return ent;
}

/*
 * Problem:
 *	remapping
 */
static void paging_map(pt_t pml4, usize va, usize pa, isize sz, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ4K));
	ASSERT(ALIGNED(pa, PG_SZ4K));
	ASSERT(ALIGNED(sz, PG_SZ4K));

	while (sz > 0) {
		// For now we strip huge pages as they are somewhat tricky.
		if (sz >= PG_SZ4K && ALIGNED(va | pa, PG_SZ4K)) {
			paging_map_4K(pml4, va, pa, flags);
			va += PG_SZ4K;
			pa += PG_SZ4K;
			sz -= PG_SZ4K;
			continue;
		}
		ASSERT(0);
	}
}

// level is in range [1, 2, 3, 4]
static pt_t pt_deepcopy(pt_t pt, int level)
{
	ASSERT(1 <= level && level <= 4);
	pt_t newpt = kalloc();

	// last level: copy content verbatim
	if (level == 1) {
		memcpy(newpt, pt, PG_SZ4K);
		return newpt;
	}

	for (int i = 0; i < PG_NENT; i++) {
		if (!(pt[i] & PF_P))
			continue;

		// huge page: copy entry verbatim
		if (pt[i] & PF_PS) {
			newpt[i] = pt[i];
			continue;
		}

		// recursively copy, reset entry addr to new next pt
		pt_t nextpt = safe_nextpt_noalloc(pt, i);
		pt_t newnextpt = pt_deepcopy(nextpt, level - 1);
		u64 ent = V2P(newnextpt) | PE_FLAGS(pt[i]);
		safe_setent(newpt, i, ent);
	}

	return newpt;
}

// Switch from bootloader's entrypml4 to kernel's pml4.
void paginginit_bsp(void)
{
	kpml4 = kalloc();

	// 0xFFFF_FFFF_8000_0000 ... 0xFFFF_FFFF_FFFF_FFFF (+2G) -> 0x0 (+2G)
	// 0xFFFF_FFFF_4000_0000 ... (+32M) -> 0xFE00_0000 ... 0xFFFF_FFFF (+32M)
	paging_map(kpml4, KERNBASE, 0, MAX_PHYS_MEM, PF_W | PF_U);
	paging_map(kpml4, DEVBASE, 0xFE000000, 16 * PG_SZ2M,
		   PF_PWT | PF_PCD | PF_U);

	lcr3(V2P(kpml4));
	cprintf("paing: bsp init\n");
}

void paginginit_ap(void)
{
	ASSERT(kpml4);
	lcr3(V2P(kpml4));
	cprintf("paing: ap init\n");
}

// TODO: different page tables for different users
void uvm_init(void)
{
	ASSERT(kpml4);

	upml4 = pt_deepcopy(kpml4, 4);
	paging_map(upml4, (usize)ubegin, V2P(ubegin), uend - ubegin,
		   PF_W | PF_U);
	cprintf("uvminit: mapped %p ~ %p as PF_U\n", ubegin, uend);
}
