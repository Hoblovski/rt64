#include "rt64u.h"

__attribute__((section(".user.text"))) void *uhello(void *_arg)
{
	const char *msg = "User hello world\n";
	usysprint(msg);
	msg += 5;
	usysprint(msg);
	usysexit();
}
