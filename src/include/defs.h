#pragma once

// kernel.ld
extern char end[];

// vectors.S
extern void *vectors[];

// entry.S
void wrmsr(u32 msr, u64 val);

// uart.c
void uartearlyinit(void);
void microdelay(int us);
void uartputc(int c);

// console.c
void cprintf(char *fmt, ...);
void panic(char *s) __attribute__((noreturn));

// acpi.c
void acpiinit(void);
extern struct percpu cpus[MAX_CPU];
extern int ncpu;
extern u8 ioapicid;

// lapic.c
void lapicinit(void);
u32 lapicid(void);
void lapiceoi(void);

// paging.c
void paginginit_bsp(void);
void paginginit_ap(void);

// trap.c
void trapinit();
