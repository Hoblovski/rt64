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
	ASSERT(len <= 33);
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

static int isdigit(char x)
{
	return x >= '0' && x <= '9';
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
		i++;

		// after % comes width specifier
		int minwidth = 0;
		while (fmt[i] && isdigit(fmt[i])) {
			minwidth = minwidth * 10 + fmt[i] - '0';
			i++;
		}
		c = fmt[i] & 0xff;
		if (c == 0)
			break;
		switch (c) {
		case 'd':
			printint((long)va_arg(ap, int), 10, 1, minwidth, ' ');
			break;
		case 'l':
			printint(va_arg(ap, long), 10, 1, minwidth, ' ');
			break;
		case 'x':
			printint((long)va_arg(ap, int), 16, 0, minwidth, ' ');
			break;
		case 'y':
			printint(va_arg(ap, long), 16, 0, minwidth, ' ');
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

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vcprintf(fmt, ap);
	va_end(ap);
}

__attribute__((noreturn)) static void vpanic(char *fmt, va_list ap)
{
	cprintf("======== panic ========\n");
	vcprintf(fmt, ap);
	cprintf("pc backtrace:\n");
	usize *rbp;
	asm volatile("mov %%rbp, %0" : "=r"(rbp));
	while (rbp[0] != 0) {
		cprintf("  %p\n", rbp[1]);
		rbp = (usize *)rbp[0];
	}

	panicked = 1;
	for (;;)
		;
}

void panic(char *fmt, ...)
{
	cli();
	va_list ap;
	va_start(ap, fmt);
	vpanic(fmt, ap);
	va_end(ap);
}

void assert(int cond, char *fmt, ...)
{
	if (cond)
		return;
	cli();

	va_list ap;
	va_start(ap, fmt);
	vpanic(fmt, ap);
	va_end(ap);
}
