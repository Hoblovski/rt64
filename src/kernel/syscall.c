#include "rt64.h"

isize syscall(isize num, usize a0, usize a1, usize a2, usize a3, usize a4,
	      usize a5)
{
	switch (num) {
	case SYS_print:
		return sys_print((const char *)a0);
	case SYS_exit:
		sys_exit();
	default:
		return -1;
	}
}

isize sys_print(const char *a0)
{
	cprintf("%s", a0);
	return 0;
}

isize sys_exit(void)
{
	exit();
}
