# KERNEL ================================
KERNPREFIX = src/kernel
KLIBPREFIX = src/klib
USERPREFIX = src/user
ULIBPREFIX = src/ulib
TOOLSPREFIX = src/tools
INCLUDEPATH = src/include

OUTPREFIX = out
KOBJPREFIX = $(OUTPREFIX)/kobj
UOBJPREFIX = $(OUTPREFIX)/uobj
IMGPREFIX = $(OUTPREFIX)
XV6IMG = $(IMGPREFIX)/xv6.img

KERNELELF = $(OUTPREFIX)/kernel.elf

OBJS := \
	$(KOBJPREFIX)/main.o\
	$(KOBJPREFIX)/uart.o\
	$(KOBJPREFIX)/console.o\
	$(KOBJPREFIX)/acpi.o\
	$(KOBJPREFIX)/lapic.o\
	$(KOBJPREFIX)/string.o

CC = gcc
AS = gas
LD = ld
NM = nm
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU ?= qemu-system-x86_64

OPTFLAGS ?= -O0
X64CFLAGS = -m64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone
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

$(KOBJPREFIX)/%.o: $(KLIBPREFIX)/%.c
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJPREFIX)/%.o: $(KERNPREFIX)/%.S
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(ASFLAGS) -c -o $@ $<

# bootblock
$(OUTPREFIX)/bootblock: $(KERNPREFIX)/bootasm.S $(KERNPREFIX)/bootmain.c
	@mkdir -p $(OUTPREFIX)
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -O -o $(OUTPREFIX)/bootmain.o -c $(KERNPREFIX)/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -o $(OUTPREFIX)/bootasm.o -c $(KERNPREFIX)/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs -N -e start -Ttext 0x7C00 -o $(OUTPREFIX)/bootblock.o $(OUTPREFIX)/bootasm.o $(OUTPREFIX)/bootmain.o
	$(OBJDUMP) -S $(OUTPREFIX)/bootblock.o > $(OUTPREFIX)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(OUTPREFIX)/bootblock.o $(OUTPREFIX)/bootblock
	$(TOOLSPREFIX)/sign.pl $(OUTPREFIX)/bootblock

# kernel elf
ENTRYCODE = $(KOBJPREFIX)/entry64.o
LINKSCRIPT = $(KERNPREFIX)/kernel64.ld
$(KERNELELF): $(OBJS) $(ENTRYCODE) $(LINKSCRIPT)
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o $(KERNELELF) $(ENTRYCODE) $(OBJS)
	$(OBJDUMP) -S $(KERNELELF) > $(OUTPREFIX)/kernel.asm
	$(OBJDUMP) -t $(KERNELELF) | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUTPREFIX)/kernel.sym

# xv6.img
$(XV6IMG): $(OUTPREFIX)/bootblock $(KERNELELF)
	@mkdir -p $(IMGPREFIX)
	dd if=/dev/zero of=$(XV6IMG) count=10000
	dd if=$(OUTPREFIX)/bootblock of=$(XV6IMG) conv=notrunc
	dd if=$(KERNELELF) of=$(XV6IMG) seek=1 conv=notrunc

# PHONY
clean:
	rm -rf $(OUTPREFIX)

format:
	find src -name "*.[ch]" | xargs clang-format -i

# QEMU and GDB ================================

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -gdb tcp::$(GDBPORT)
CPUS ?= 2
QEMUOPTS = -net none -hda $(XV6IMG) -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: $(XV6IMG)
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-nox: $(XV6IMG)
	$(QEMU) -nographic $(QEMUOPTS)

.gdbinit: $(TOOLSPREFIX)/gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: $(XV6IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: $(XV6IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

# SERVAL ================================
#
VERIFY_TEST := \
	verif/test.rkt

SERVAL_DIR = ../serval
SERVAL_LLVM := racket $(SERVAL_DIR)/serval/bin/serval-llvm.rkt

RKTGENPREFIX = verif/generated
ASMRKTGEN = $(RKTGENPREFIX)/kernel.asm.rkt
GLOBALRKTGEN = $(RKTGENPREFIX)/kernel.global.rkt
MAPRKTGEN = $(RKTGENPREFIX)/kernel.map.rkt

RACO_JOBS = 1
RACO_TIMEOUT = 1200
RACO_TEST = raco test --check-stderr --table --timeout $(RACO_TIMEOUT) --jobs $(RACO_JOBS)

$(VERIFY_TEST): | \
	$(ASMRKTGEN) \
	$(GLOBALRKTGEN) \
	$(MAPRKTGEN)

verify-kernel: $(VERIFY_TEST)
	$(RACO_TEST) $^

$(GLOBALRKTGEN): $(KERNELELF)
	@mkdir -p $(RKTGENPREFIX)
	@echo "#lang reader serval/lang/dwarf" > $@~
	@$(OBJDUMP) --dwarf=info $< >> $@~
	@mv $@~ $@

ASMRKTGEN_OBJDUMPFLAGS = -M no-aliases --prefix-address -w -f -d -z --show-raw-insn
$(ASMRKTGEN): $(KERNELELF)
	@mkdir -p $(RKTGENPREFIX)
	@echo "#lang reader serval/riscv/objdump" > $@~
	@$(OBJDUMP) $(ASMRKTGEN_OBJDUMPFLAGS) $< >> $@~
	@mv $@~ $@

$(MAPRKTGEN): $(KERNELELF)
	@mkdir -p $(RKTGENPREFIX)
	@echo "#lang reader serval/lang/nm" > $@~
	@$(NM) --print-size --numeric-sort $< >> $@~
	@mv $@~ $@


