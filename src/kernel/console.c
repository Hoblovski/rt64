// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "rt64.h"

#include <stdarg.h>

static void consputc(int);

static int panicked = 0;
static char digits[] = "0123456789abcdef";

static void printptr(void *ptr)
{
	u64 x = (u64)ptr;
	char s[16];
	int sl = 0;
	for (int i = 0; i < XLENB * 2; i++) {
		s[sl++] = digits[x & 15];
		x >>= 4;
	}
	for (int i = sl - 1; i >= 0; i--)
		consputc(s[i]);
}

static void printint(int xx, int base, int sign)
{
	char buf[16];
	int i;
	usize x;

	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (sign)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

#define BACKSPACE 0x100
static void consputc(int c)
{
	if (panicked) {
		cli();
		for (;;)
			;
	}

	if (c == BACKSPACE) {
		uartputc('\b');
		uartputc(' ');
		uartputc('\b');
	} else
		uartputc(c);
}

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char *fmt, ...)
{
	va_list ap;
	int i, c;
	char *s;

	va_start(ap, fmt);

	// TODO: on MP, check locking console
	if (fmt == 0)
		panic("null fmt");

	for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
		if (c != '%') {
			consputc(c);
			continue;
		}
		c = fmt[++i] & 0xff;
		if (c == 0)
			break;
		switch (c) {
		case 'd':
			printint(va_arg(ap, int), 10, 1);
			break;
		case 'x':
			printint(va_arg(ap, int), 16, 0);
			break;
		case 'p':
			printptr(va_arg(ap, void *));
			break;
		case 's':
			if ((s = va_arg(ap, char *)) == 0)
				s = "(null)";
			for (; *s; s++)
				consputc(*s);
			break;
		case '%':
			consputc('%');
			break;
		default:
			// Print unknown % sequence to draw attention.
			consputc('%');
			consputc(c);
			break;
		}
	}
}

void panic(char *s)
{
	cli();
	cprintf("panic: ");
	cprintf(s);
	cprintf("\n");
	panicked = 1;
	for (;;)
		;
}
