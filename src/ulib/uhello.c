#include "rt64u.h"

__attribute__((section(".user.bss")))
static int a = 6;

__attribute__((section(".user.text")))
void *uhello(void *_arg)
{
	const char *msg = "User hello world\n";
	extern int print(const char *p);
	print(msg);
	return a; // NULL;
}
