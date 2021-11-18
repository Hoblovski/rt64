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

# BUGS
For now it's a little messy on `int/unsigned/u32/u64/usize`.

# ACKNOWLEDGEMENT
Much code is adopted (or, copied) from xv6 and swetland/xv6.
