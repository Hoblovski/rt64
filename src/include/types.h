#pragma once

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;
typedef long isize;
typedef unsigned long usize;

// TODO: this is confusing
#ifdef STATIC_ASSERT
STATIC_ASSERT(sizeof(i16) == 2, bad_platform);
STATIC_ASSERT(sizeof(u16) == 2, bad_platform);
STATIC_ASSERT(sizeof(i32) == 4, bad_platform);
STATIC_ASSERT(sizeof(u32) == 4, bad_platform);
STATIC_ASSERT(sizeof(i64) == 8, bad_platform);
STATIC_ASSERT(sizeof(u64) == 8, bad_platform);
STATIC_ASSERT(sizeof(isize) == XLENB, bad_platform);
STATIC_ASSERT(sizeof(usize) == XLENB, bad_platform);
#endif

// Per-CPU state
struct percpu {
	// cpuid: normalized (i.e. successive numbers from 0) based on lapicid
	// INVARIANT: forall i, cpus[i].index = i
	u8 index;
	// Local APIC ID
	u8 lapicid;
	// TLS storage, holds GDT and TSS
	void *tls;
};

struct trapframe;
