#pragma once
// memory layout

#define EXTMEM 0x100000 // pa: Start of extended memory
#define PHYSTOP 0xE000000 // pa: Top physical memory
#define DEVSPACE 0xFE000000 // pa: Other devices are at high addresses

#define KERNBASE                                                               \
	0xFFFFFFFF80000000 // va: FFFF_FFFF_8000_000, start va of kernel space (actual kernel code starts from KERNBASE + EXTMEM)
#define DEVBASE                                                                \
	0xFFFFFFFF40000000 // va: FFFF_FFFF_4000_000, start va of high address devices

// Returns an integer
#define V2P(a) (((usize)(a)) - KERNBASE)
// Returns a pointer
#define P2V(a) (((void *)(a)) + KERNBASE)
// Only for high-address devices like *APIC. Returns a pointer
#define IO2V(a) (((void *)(a)) - DEVSPACE + DEVBASE)
