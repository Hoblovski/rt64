#include "rt64.h"

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static u16 *crt = (u16 *)P2V(0xb8000); // CGA memory

void cgaputc(int c)
{
	int pos;

	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT + 1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT + 1);

	if (c == '\n')
		pos += 80 - pos % 80;
	else if (c == BACKSPACE) {
		if (pos > 0)
			--pos;
	} else
		crt[pos++] = (c & 0xff) | 0x0700; // black on white

	if ((pos / 80) >= 24) { // Scroll up.
		memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
		pos -= 80;
		memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
	}

	outb(CRTPORT, 14);
	outb(CRTPORT + 1, pos >> 8);
	outb(CRTPORT, 15);
	outb(CRTPORT + 1, pos);
	crt[pos] = ' ' | 0x0700;
}
