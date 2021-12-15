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
	ASSERT(!(pt[idx] & PF_P));
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
	safe_setent(pt, idx, V2P(nextpt) | PF_W | PF_P | PF_U);
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

static void paging_map_4K(pt_t pml4, usize va, usize pa, usize flags)
{
	ASSERT(ALIGNED(va, PG_SZ4K));
	ASSERT(ALIGNED(pa, PG_SZ4K));

	pt_t pdpt = safe_nextpt_mayalloc(pml4, PML4_IDX(va));
	pt_t pd = safe_nextpt_mayalloc(pdpt, PDPT_IDX(va));
	pt_t pt = safe_nextpt_mayalloc(pd, PD_IDX(va));
	u64 ent = pa | flags | PF_P;

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
 * NOTE: Does not check remapping, could destroy previous mappings.
 */
void paging_map(pt_t pml4, usize va, usize pa, isize sz, usize flags)
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

static pt_t pt_deepcopy_level(pt_t pt, int level)
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
		pt_t newnextpt = pt_deepcopy_level(nextpt, level - 1);
		u64 ent = V2P(newnextpt) | PE_FLAGS(pt[i]);
		safe_setent(newpt, i, ent);
	}

	return newpt;
}

/*
 * Deep copy an address space
 */
pt_t pt_root_deepcopy(pt_t pt)
{
	return pt_deepcopy_level(pt, 4);
}

// Switch from bootloader's entrypml4 to kernel's pml4.
void paginginit_bsp(void)
{
	kpml4 = kalloc();

	// 0xFFFF_FFFF_8000_0000 ... 0xFFFF_FFFF_FFFF_FFFF (+2G) -> 0x0 (+2G)
	// 0xFFFF_FFFF_4000_0000 ... (+32M) -> 0xFE00_0000 ... 0xFFFF_FFFF (+32M)
	paging_map(kpml4, KERNBASE, 0, MAX_PHYS_MEM, PF_W);
	paging_map(kpml4, DEVBASE, 0xFE000000, 16 * PG_SZ2M, PF_PWT | PF_PCD);

	lcr3(V2P(kpml4));
	cprintf("paing: bsp init\n");
}

void paginginit_ap(void)
{
	ASSERT(kpml4);
	lcr3(V2P(kpml4));
	cprintf("paging: ap init\n");
}

// TODO: different page tables for different users
void uvminit(void)
{
	ASSERT(kpml4);

	upml4 = pt_root_deepcopy(kpml4);
	paging_map(upml4, (usize)ubegin, V2P(ubegin), uend - ubegin,
		   PF_W | PF_U);
	cprintf("uvminit: mapped %p ~ %p as PF_U\n", ubegin, uend);
}
