#pragma once

#define XLEN 64
#define XLENB 8
#define MAX_CPU 8

// From experience, a period of 1 000 000 000 is equal to 1 second on qemu.
#define LAPIC_TIMER_PERIOD 10000000
