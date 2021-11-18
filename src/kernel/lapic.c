#include "rt64.h"

// detected and initialized in acpi.c, points to physical addr 0xFEE0_0000
volatile u32 *lapic;
