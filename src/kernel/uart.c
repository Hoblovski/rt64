/*
 * Driver for Intel 8250 serial port (UART).
 */

#include "rt64.h"

#define COM1 0x3f8

static int uart; // is there a uart?

void uartearlyinit(void)
{
	char *p;

	// Turn off the FIFO
	outb(COM1 + 2, 0);

	// 9600 baud, 8 data bits, 1 stop bit, parity off.
	outb(COM1 + 3, 0x80); // Unlock divisor
	outb(COM1 + 0, 115200 / 9600);
	outb(COM1 + 1, 0);
	outb(COM1 + 3, 0x03); // Lock divisor, 8 data bits.
	outb(COM1 + 4, 0);
	outb(COM1 + 1, 0x01); // Enable receive interrupts.

	// If status is 0xFF, no serial port.
	if (inb(COM1 + 5) == 0xFF)
		return;
	uart = 1;

	// Announce that we're here.
	for (p = "\nuart: earlyinit\n"; *p; p++)
		uartputc(*p);
}

void microdelay(int us)
{
	// on real hardware tune this dynamically
}

void uartputc(int c)
{
	int i;

	if (!uart)
		return;
	for (i = 0; i < 128 && !(inb(COM1 + 5) & 0x20); i++)
		microdelay(10);
	outb(COM1 + 0, c);
}
