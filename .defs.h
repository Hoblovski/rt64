#pragma once

/*
 * sched: print debug info
 */
#define DEBUG_SCHED

/*
 * trap: print tick info every CONFIG_TICK_INTERVAL ticks
 */
#define DEBUG_TICK
#define CONFIG_TICK_INTERVAL 10

/*
 * Pool for ready thread rather than WFI when idle.
 */
#define CONFIG_IDLE_POOL
