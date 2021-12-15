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
	uvminit(); // XXX: .


	extern void *uhello(void *arg);
	spawnuser(& (struct newprocdesc) {
			.name = "uhello",
			.func = uhello,
			.initarg = NULL,
			.prio = 0
	});

	extern void *minctestmain(void *arg);
	spawn(& (struct newprocdesc) {
			.name = "master",
			.func = minctestmain,
			.initarg = NULL,
			.prio = 0
	});

	idlemain();
}

void mpenter(void)
{
}
