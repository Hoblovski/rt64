#pragma once

// uart.c
void uartearlyinit(void);
void microdelay(int us);
void uartputc(int c);

// console.c
void cprintf(char *fmt, ...);
void panic(char *s) __attribute__((noreturn));

// acpi.c
void acpiinit(void);
extern struct cpu cpus[MAX_CPU];
extern int ncpu;
extern int ismp;
extern u8 ioapicid;

// lapic.c
extern volatile u32 *lapic;
