#pragma once

/*
 * sched: print debug info
 */
#undef DEBUG_SCHED

/*
 * trap: print tick info every CONFIG_TICK_INTERVAL ticks
 */
#undef DEBUG_TICK
#define CONFIG_TICK_INTERVAL 10

/*
 * Pool for ready thread rather than WFI when idle.
 */
#undef CONFIG_IDLE_POOL

/*
 * Skip all ACPI checks and manually specify ACPI info.
 * This is used for booting on T440.
 */
#define CONFIG_DUMMY_ACPI
#define CONFIG_DUMMY_ACPI_NCPUS 1
