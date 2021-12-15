# KERNEL

# SETUP
Requirements
* `qemu-system-x86_64`

Just
```bash
make src/kernel/vectors.S

make qemu-nox
```

# DOCS
Exec flow:
```
-> bootasm.S -> bootmain.c		[BOOTLOADER]
-> entry64.S				[BSP init]
-> main.c:main				[BSP init]
```

Mem upon boot (i.e. entry to main) is set by `mboot_entry` in `entry64.S` and mapped as follows
```
0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF (+1G)	->	0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF
	by a single 1G huge page

FFFF_FFFF_8000_0000 ... FFFF_FFFF_BFFF_FFFF (+1G)	->	0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF
	by a single 1G huge page
```

As we have 1G max phys memory, so they're all mapped by boot code.

With PML4 page table at PA 0x1000.

The kernel and user code are linked (literally) together, which means you can see kernel's symbols from user space.
But thanks to protection offered by page table, user code may not access them directly.

# Dev
## Source hierarchy
```
src
├── tools
	tool scripts for building the kernel

├── include
	headers, both user and kenrel.
	yet kernel c files should solely
		#include "rt64.h"
	and user c files solely
		#include "rt64u.h"
	and asm files solely
		#include "rt64asm.h"
	all others should be indirectly included

├── kernel
	the rt64 kernel itself.

├── kapp
	apps run in kernel mode, and calls the kernel's routines..
		#include "rt64.h"

├── klib
	generic libraries for the kernel e.g. memcpy

├── uapp
	apps run in user mode, invokes syscalls for services.
		#include "rt64u.h"

└── ulib
	libraries for user code.
```

# BUGS
For now it's a little messy on `int/unsigned/u32/u64/usize`.

reverse allocation

# TODO
Should we strip dynamic allocation?

# Thoughts
The good old 'same code runs in user / kernel' idea?

# DEBUG
## addr2line
Just feed the pc addr dumped by panic to `make addr2line`.
Note that rip is the next instruction to execute so you may need to subtract line numbers by one.

## preprocessing
`make out/main.i`

## gdb
Better install cgdb

```
make qemu-nox-gdb

# in another window, CWD = rt64/
cgdb
	# tb main
	# c
```

# ACKNOWLEDGEMENT
Much code is adopted (or, copied) from xv6 and swetland/xv6.

