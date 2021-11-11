KERNPREFIX=src/kernel
USERPREFIX=src/user
ULIBPREFIX=src/ulib
TOOLSPREFIX=src/tools
INCLUDEPATH=src/include

OUTPREFIX=out
KOBJPREFIX=$(OUTPREFIX)/kobj
UOBJPREFIX=$(OUTPREFIX)/uobj
FSPREFIX=fs
IMGPREFIX=$(OUTPREFIX)
FSIMG=$(IMGPREFIX)/fs.img
MEMFSIMG=$(IMGPREFIX)/xv6memfs.img
XV6IMG=$(IMGPREFIX)/xv6.img

OBJS := \
	$(KOBJPREFIX)/bio.o\
	$(KOBJPREFIX)/console.o\
	$(KOBJPREFIX)/exec.o\
	$(KOBJPREFIX)/file.o\
	$(KOBJPREFIX)/fs.o\
	$(KOBJPREFIX)/ide.o\
	$(KOBJPREFIX)/ioapic.o\
	$(KOBJPREFIX)/kalloc.o\
	$(KOBJPREFIX)/kbd.o\
	$(KOBJPREFIX)/lapic.o\
	$(KOBJPREFIX)/log.o\
	$(KOBJPREFIX)/main.o\
	$(KOBJPREFIX)/mp.o\
	$(KOBJPREFIX)/acpi.o\
	$(KOBJPREFIX)/picirq.o\
	$(KOBJPREFIX)/pipe.o\
	$(KOBJPREFIX)/proc.o\
	$(KOBJPREFIX)/spinlock.o\
	$(KOBJPREFIX)/string.o\
	$(KOBJPREFIX)/swtch64.o\
	$(KOBJPREFIX)/syscall.o\
	$(KOBJPREFIX)/sysfile.o\
	$(KOBJPREFIX)/sysproc.o\
	$(KOBJPREFIX)/timer.o\
	$(KOBJPREFIX)/trapasm64.o\
	$(KOBJPREFIX)/trap.o\
	$(KOBJPREFIX)/uart.o\
	$(KOBJPREFIX)/vectors.o\
	$(KOBJPREFIX)/vm.o\
	$(KOBJPREFIX)/vm64.o

ifneq ("$(MEMFS)","")
# build filesystem image in to kernel and use memory-ide-device
# instead of mounting the filesystem on ide1
OBJS := $(filter-out $(KOBJPREFIX)/ide.o,$(OBJS)) $(KOBJPREFIX)/memide.o
FSIMAGE := $(FSIMG)
endif

CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU ?= qemu-system-x86_64

OPTFLAGS ?= -O0
X64CFLAGS = -m64 -DX64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -MD -ggdb -fno-omit-frame-pointer
CFLAGS += -ffreestanding -fno-common -nostdlib -I$(INCLUDEPATH) -gdwarf-2 $(X64CFLAGS) $(OPTFLAGS)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -fno-pic -gdwarf-2 -Wa,-divide -I$(INCLUDEPATH) $(X64CFLAGS)
LDFLAGS = -m elf_x86_64 -nodefaultlibs

all: $(XV6IMG)

# kernel object files
$(KOBJPREFIX)/%.o: $(KERNPREFIX)/%.c
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJPREFIX)/%.o: $(KERNPREFIX)/%.S
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(ASFLAGS) -c -o $@ $<

# userspace object files
$(UOBJPREFIX)/%.o: $(USERPREFIX)/%.c
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(UOBJPREFIX)/%.o: $(ULIBPREFIX)/%.c
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(UOBJPREFIX)/%.o: $(ULIBPREFIX)/%.S
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(OUTPREFIX)/bootblock: $(KERNPREFIX)/bootasm.S $(KERNPREFIX)/bootmain.c
	@mkdir -p $(OUTPREFIX)
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -O -o $(OUTPREFIX)/bootmain.o -c $(KERNPREFIX)/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -o $(OUTPREFIX)/bootasm.o -c $(KERNPREFIX)/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs -N -e start -Ttext 0x7C00 -o $(OUTPREFIX)/bootblock.o $(OUTPREFIX)/bootasm.o $(OUTPREFIX)/bootmain.o
	$(OBJDUMP) -S $(OUTPREFIX)/bootblock.o > $(OUTPREFIX)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(OUTPREFIX)/bootblock.o $(OUTPREFIX)/bootblock
	$(TOOLSPREFIX)/sign.pl $(OUTPREFIX)/bootblock

$(OUTPREFIX)/entryother: $(KERNPREFIX)/entryother.S
	@mkdir -p $(OUTPREFIX)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -o $(OUTPREFIX)/entryother.o -c $<
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $(OUTPREFIX)/bootblockother.o $(OUTPREFIX)/entryother.o
	$(OBJCOPY) -S -O binary -j .text $(OUTPREFIX)/bootblockother.o $(OUTPREFIX)/entryother
	$(OBJDUMP) -S $(OUTPREFIX)/bootblockother.o > $(OUTPREFIX)/entryother.asm

$(OUTPREFIX)/initcode: $(KERNPREFIX)/initcode64.S
	@mkdir -p $(OUTPREFIX)
	$(CC) $(CFLAGS) -nostdinc -I. -o $(OUTPREFIX)/initcode.o -c $<
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(OUTPREFIX)/initcode.out $(OUTPREFIX)/initcode.o
	$(OBJCOPY) -S -O binary $(OUTPREFIX)/initcode.out $(OUTPREFIX)/initcode
	$(OBJDUMP) -S $(OUTPREFIX)/initcode.o > $(OUTPREFIX)/initcode.asm


ENTRYCODE = $(KOBJPREFIX)/entry64.o
LINKSCRIPT = $(KERNPREFIX)/kernel64.ld
$(OUTPREFIX)/kernel.elf: $(OBJS) $(ENTRYCODE) $(OUTPREFIX)/entryother $(OUTPREFIX)/initcode $(LINKSCRIPT) $(FSIMAGE)
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o $(OUTPREFIX)/kernel.elf $(ENTRYCODE) $(OBJS) -b binary $(OUTPREFIX)/initcode $(OUTPREFIX)/entryother $(FSIMAGE)
	$(OBJDUMP) -S $(OUTPREFIX)/kernel.elf > $(OUTPREFIX)/kernel.asm
	$(OBJDUMP) -t $(OUTPREFIX)/kernel.elf | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUTPREFIX)/kernel.sym

$(KERNPREFIX)/vectors.S: $(TOOLSPREFIX)/vectors64.pl
	perl $< > $(KERNPREFIX)/vectors.S

ULIB = $(UOBJPREFIX)/ulib.o $(UOBJPREFIX)/usys.o $(UOBJPREFIX)/printf.o $(UOBJPREFIX)/umalloc.o

$(FSPREFIX)/%: $(UOBJPREFIX)/%.o $(ULIB)
	@mkdir -p $(FSPREFIX)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $(OUTPREFIX)/$*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUTPREFIX)/$*.sym

$(FSPREFIX)/forktest: $(UOBJPREFIX)/forktest.o $(ULIB)
	@mkdir -p $(FSPREFIX)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $(FSPREFIX)/forktest $(UOBJPREFIX)/forktest.o $(UOBJPREFIX)/ulib.o $(UOBJPREFIX)/usys.o
	$(OBJDUMP) -S $(FSPREFIX)/forktest > $(OUTPREFIX)/forktest.asm

$(OUTPREFIX)/mkfs: $(TOOLSPREFIX)/mkfs.c $(INCLUDEPATH)/fs.h
	gcc -Werror -Wall -o $(OUTPREFIX)/mkfs $(TOOLSPREFIX)/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: $(UOBJPREFIX)/%.o

UPROGS=\
	$(FSPREFIX)/cat\
	$(FSPREFIX)/echo\
	$(FSPREFIX)/forktest\
	$(FSPREFIX)/grep\
	$(FSPREFIX)/init\
	$(FSPREFIX)/kill\
	$(FSPREFIX)/ln\
	$(FSPREFIX)/ls\
	$(FSPREFIX)/mkdir\
	$(FSPREFIX)/rm\
	$(FSPREFIX)/sh\
	$(FSPREFIX)/stressfs\
	$(FSPREFIX)/usertests\
	$(FSPREFIX)/wc\
	$(FSPREFIX)/zombie

$(FSIMG): $(OUTPREFIX)/mkfs README.md $(UPROGS)
	@mkdir -p $(IMGPREFIX)
	$(OUTPREFIX)/mkfs $(FSIMG) README.md $(UPROGS)

$(XV6IMG): $(OUTPREFIX)/bootblock $(OUTPREFIX)/kernel.elf $(FSIMG)
	@mkdir -p $(IMGPREFIX)
	dd if=/dev/zero of=$(XV6IMG) count=10000
	dd if=$(OUTPREFIX)/bootblock of=$(XV6IMG) conv=notrunc
	dd if=$(OUTPREFIX)/kernel.elf of=$(XV6IMG) seek=1 conv=notrunc

$(MEMFSIMG): $(OUTPREFIX)/bootblock $(OUTPREFIX)/kernelmemfs.elf
	@mkdir -p $(IMGPREFIX)
	dd if=/dev/zero of=$(MEMFSIMG) count=10000
	dd if=$(OUTPREFIX)/bootblock of=$(MEMFSIMG) conv=notrunc
	dd if=$(OUTPREFIX)/kernelmemfs.elf of=$(MEMFSIMG) seek=1 conv=notrunc

clean:
	rm -rf $(OUTPREFIX) $(FSPREFIX)


# GDB ================================

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -gdb tcp::$(GDBPORT)
CPUS ?= 2
QEMUOPTS = -net none -hda $(XV6IMG) -hdb $(FSIMG) -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: $(FSIMG) $(XV6IMG)
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-memfs: $(MEMFSIMG)
	$(QEMU) $(MEMFSIMG) -smp $(CPUS)

qemu-nox: $(FSIMG) $(XV6IMG)
	$(QEMU) -nographic $(QEMUOPTS)

.gdbinit: $(TOOLSPREFIX)/gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(FSIMG) $(XV6IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: $(FSIMG) $(XV6IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

