# KERNEL

# SETUP

Just
```bash
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

Should we strip dynamic allocation?

# BUGS
For now it's a little messy on `int/unsigned/u32/u64/usize`.

Makefile does not account for changes in .h files.

reverse allocation

user init

# DEBUG
## addr2line
Just feed the pc addr dumped by panic to `make addr2line`.
Note that rip is the next instruction to execute so you may need to subtract line numbers by one.

# ACKNOWLEDGEMENT
Much code is adopted (or, copied) from xv6 and swetland/xv6.

