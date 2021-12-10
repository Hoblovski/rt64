#include "rt64u.h"

__attribute__((section(".user.text"))) void *uhello(void *_arg)
{
	const char *msg = "User hello world\n";
	extern int print(const char *p);
	print(msg);
	msg += 5;
	print(msg);
	while (1)
		;

	return NULL;
}
