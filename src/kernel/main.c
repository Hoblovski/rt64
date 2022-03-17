#include "rt64.h"

// BSP starts here
int main(void)
{
	uartearlyinit();
	kallocinit();
	paginginit_bsp();
	acpiinit();

	lapicinit();
	trapinit();
	procinit();
	uvminit();

	extern void *minctestmain(void *arg);
	spawn(&(struct newprocdesc){ .name = "master",
				     .func = minctestmain,
				     .initarg = NULL,
				     .prio = 0 });
	idlemain();
}

void mpenter(void)
{
	paginginit_ap();
	trapinit();
	xchg64((isize *)&curcpu->started, 1); // tell startothers() we're up
	while (1)
		;
}
