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

With PML4 page table at PA 0x1000.

# BUGS
For now it's a little messy on `int/unsigned/u32/u64/usize`.

# DEBUG
## addr2line
Add a NPD on main, and it'll show

```
SeaBIOS (version 1.13.0-1ubuntu1.1)
Booting from Hard Disk..
uart: earlyinit
paing: bsp init
acpi: sig=FACP id=BOCHS  tableid=BXPCFACP oemrev=1 creat=BXPC creatrev=1
acpi: sig=APIC id=BOCHS  tableid=BXPCAPIC oemrev=1 creat=BXPC creatrev=1
acpi: sig=HPET id=BOCHS  tableid=BXPCHPET oemrev=1 creat=BXPC creatrev=1
acpi: cpu#0 apicid 0
acpi: ioapic#0 @fffffffffec00000 id=0 base=0
[0] lapic: init
trap: init
======== panic ========
unexpected trap 14 from cpu 0 rip ffffffff80100143 (cr2=0x0)
QEMU: Terminated
```

Then

```
$ addr2line -e kernel.elf ffffffff80100143
src/kernel/main.c:12
```

# ACKNOWLEDGEMENT
Much code is adopted (or, copied) from xv6 and swetland/xv6.

