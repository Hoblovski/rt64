# KERNEL
From swetland/xv6.git, the 64-bit xv6.

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

Mem upon boot (i.e. entry to main) is set by `mboot_entry` and mapped as follows
```
0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF (+1G)	->	0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF
	by 512 huge pages each is 2M

FFFF_FFFF_8000_0000 ... FFFF_FFFF_BFFF_FFFF (+1G)	->	0000_0000_0000_0000 ... 0000_0000_3FFF_FFFF
	by 512 huge pages each is 2M
```

# BUGS
For now it's a little messy on `int/unsigned/u32/u64/usize`.

