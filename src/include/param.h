#pragma once

#define XLEN 64
#define XLENB 8
#define MAX_CPU 8

#define MAX_PHYS_MEM 0x1000000 // 16M
#define MAX_PROC 8
#define KSTACK_PAGES 1
#define USTACK_PAGES 1

// From experience, a period of 1 000 000 000 is equal to 1 second on qemu.
#define LAPIC_TIMER_PERIOD 10000000 // 10ms

// Empirical result
#define LAPIC_FREQ 1000000000
#define TSC_FREQ 2600000000
// Convert TSC interval to LAPIC interval:	tsc * (LAPIC_FREQ / TSC_FREQ)
// in general it could overflow, but in our case it wont
#define TSC_TO_LAPIC(i) ((i)*5 / 13)

#define MINPRIO 0
#define MAXPRIO 255

// Below are computed, do not modify
#define KSTACK_SZ (PG_SZ4K * KSTACK_PAGES)
#define USTACK_SZ (PG_SZ4K * USTACK_PAGES)
