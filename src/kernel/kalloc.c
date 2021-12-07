/*
 * Page allocation.
 *
 * Does not require the 2-stage allocator because all physical memory has been
 * mapped by bootloader.
 */

#include "rt64.h"

struct freeframe {
	struct freeframe *next;
};

static struct {
	int nfreepages;
	struct freeframe *freelist;
} allocator = {
	// TODO: locking on SMP
	.nfreepages = 0,
	.freelist = NULL
};

void kfree(void *frame)
{
	ASSERT(ALIGNED(frame, PG_SZ4K));

	struct freeframe *f = frame;
	f->next = allocator.freelist;
	allocator.freelist = f;
	allocator.nfreepages++;
}

void *kalloc(void)
{
	ASSERT(allocator.freelist != NULL);

	struct freeframe *f = allocator.freelist;
	allocator.freelist = allocator.freelist->next;
	allocator.nfreepages--;
	return (void *)f;
}

void kallocinit(void)
{
	void *from = end, *to = P2V((void *)MAX_PHYS_MEM);
	cprintf("%x %x\n", from, to);
	for (void *it = from; it < to; it += PG_SZ4K)
		kfree(it);
	cprintf("kalloc: inited %d free pages [%x - %x]\n",
		allocator.nfreepages, from, to);
}
