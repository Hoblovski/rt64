/* Console input and output.
 * Input is from the keyboard or serial port.
 * Output is written to the CGA screen (TODO) and serial port.
 */

#include "rt64.h"

#include <stdarg.h>

static void consputc(int);

static int panicked = 0;
static char digits[] = "0123456789abcdef";

static void printint(isize xx, long base, int sign, int len, char prefix)
{
	char buf[34];

	usize x;
	if (sign && (sign = xx < 0))
		x = -xx;
	else
		x = xx;

	int i = 0;
	do {
		buf[i++] = digits[x % base];
	} while ((x /= base) != 0);

	if (len != 0 && i < len) {
		// need to pad prefix
		ASSERT(prefix != 0);
		while (i < len)
			buf[i++] = prefix;
		if (sign)
			buf[i - 1] = '-';
	} else if (sign)
		buf[i++] = '-';

	while (--i >= 0)
		consputc(buf[i]);
}

static void printptr(void *ptr)
{
	printint((isize)ptr, 16, 0, 16, '0'); // will automatically sign-extend
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
	va_start(ap, fmt);
	vcprintf(fmt, ap);
	va_end(ap);
}

void vcprintf(char *fmt, va_list ap)
{
	int i, c;
	char *s;

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
			printint((long)va_arg(ap, int), 10, 1, 0, 0);
			break;
		case 'l':
			printint(va_arg(ap, long), 10, 1, 0, 0);
			break;
		case 'x':
			printint((long)va_arg(ap, int), 16, 0, 0, 0);
			break;
		case 'y':
			printint(va_arg(ap, long), 16, 0, 0, 0);
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

void panic(char *fmt, ...)
{
	cli();
	cprintf("======== panic ========\n");
	va_list ap;
	va_start(ap, fmt);
	vcprintf(fmt, ap);
	va_end(ap);
	panicked = 1;
	for (;;)
		;
}
