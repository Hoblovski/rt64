#pragma once

// uart.c
void uartearlyinit(void);
void microdelay(int us);
void uartputc(int c);

// console.c
void cprintf(char *fmt, ...);
void panic(char *s);
