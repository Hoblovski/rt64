#include "rt64u.h"

void *uhello(void *_arg)
{
	const char *msg = "User hello world\n";
	extern int print(const char *p);
	print(msg);
	return NULL;
}
