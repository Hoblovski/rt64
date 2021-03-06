#pragma once
// memory layout

#define EXTMEM 0x100000 // pa: Start of extended memory
#define PHYSTOP 0xE000000 // pa: Top physical memory
#define DEVSPACE 0xFE000000 // pa: Other devices are at high addresses

// va: FFFF_FFFF_8000_000, start va of kernel space (actual kernel code starts from KERNBASE + EXTMEM)
#define KERNBASE 0xFFFFFFFF80000000
// va: FFFF_FFFF_4000_000, start va of high address devices
#define DEVBASE 0xFFFFFFFF40000000

// Returns an integer
#define V2P(a) (((usize)(a)) - KERNBASE)
// Returns a pointer
#define P2V(a) (((void *)((usize)(a))) + KERNBASE)
// Only for high-address devices like *APIC. Returns a pointer
#define IO2V(a) (((void *)((usize)(a))) - DEVSPACE + DEVBASE)

// Whether pointer a is aligned. pgsize must be power of 2
#define ALIGNED(a, pgsize) (!(((u64)(a)) & ((pgsize)-1)))
// Align pointer a down. pgsize must be power of 2
#define ALIGN(a, pgsize) ((void *)(((u64)(a)) & (~((pgsize)-1))))

#define __page_align __attribute__((__aligned__(PG_SZ4K)));
