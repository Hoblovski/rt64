#include "rt64u.h"

__attribute__((section(".user.text"))) void *uhello(void *_arg)
{
	const char *msg = "User hello world\n";
	// NOTE: msg is in kernel space!
	//	Holding a handle to it is fine.
	//	So this function works fine, but not the following commented code.
	//
	//*(volatile char*) &msg[0];
	usysprint(msg);
	msg += 5;
	usysprint(msg);
	usysexit();
}

__attribute__((section(".user.text"))) void *ureadkern(void *addr)
{
	*(volatile int*) addr;
	return NULL;
}
