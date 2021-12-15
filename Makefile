# KERNEL ================================
KERNPREFIX = src/kernel
KLIBPREFIX = src/klib
KAPPPREFIX = src/kapp
UAPPPREFIX = src/uapp
ULIBPREFIX = src/ulib
TOOLSPREFIX = src/tools
BOOTPREFIX = src/boot
INCLUDEPATH = src/include

OUTPREFIX = out
KOBJPREFIX = $(OUTPREFIX)/kobj
UOBJPREFIX = $(OUTPREFIX)/uobj
BOOTOBJPREFIX = $(OUTPREFIX)/boot
IMGPREFIX = $(OUTPREFIX)
XV6IMG = $(IMGPREFIX)/xv6.img
KERNELELF = $(OUTPREFIX)/kernel.elf


# need to exclude entry
OBJS := \
	$(patsubst $(KERNPREFIX)/%.c,$(KOBJPREFIX)/%.o,$(wildcard $(KERNPREFIX)/*.c)) \
	$(patsubst $(KERNPREFIX)/%.S,$(KOBJPREFIX)/%.o,$(wildcard $(KERNPREFIX)/*.S)) \
	$(patsubst $(KLIBPREFIX)/%.c,$(KOBJPREFIX)/%.o,$(wildcard $(KLIBPREFIX)/*.c)) \
	$(patsubst $(KAPPPREFIX)/%.c,$(KOBJPREFIX)/%.o,$(wildcard $(KAPPPREFIX)/*.c)) \
	\
	$(patsubst $(UAPPPREFIX)/%.c,$(UOBJPREFIX)/%.o,$(wildcard $(UAPPPREFIX)/*.c)) \
	$(patsubst $(ULIBPREFIX)/%.c,$(UOBJPREFIX)/%.o,$(wildcard $(ULIBPREFIX)/*.c)) \
	$(patsubst $(ULIBPREFIX)/%.S,$(UOBJPREFIX)/%.o,$(wildcard $(ULIBPREFIX)/*.S)) \

# when linking, entry code must come in the very beginning
OBJS := $(filter-out $(KOBJPREFIX)/entry64.o,$(OBJS))

CC = gcc
AS = gas
LD = ld
NM = nm
OBJCOPY = objcopy
OBJDUMP = objdump
QEMU ?= qemu-system-x86_64

OPTFLAGS ?= -O0
X64CFLAGS = -m64 -mcmodel=kernel -mtls-direct-seg-refs -mno-red-zone
CFLAGS ?= -MMD
CFLAGS += -fno-pic -static -fno-builtin -fno-strict-aliasing -Wall -MD -ggdb -fno-omit-frame-pointer
CFLAGS += -ffreestanding -fno-common -nostdlib -I$(INCLUDEPATH) -gdwarf-2 $(X64CFLAGS) $(OPTFLAGS)
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -fno-pic -gdwarf-2 -Wa,-divide -I$(INCLUDEPATH) $(X64CFLAGS)
LDFLAGS = -m elf_x86_64 -nodefaultlibs

all: $(XV6IMG)

# generate .d files so when re-make headers are considered
-include $(OBJS:.o=.d)

# preprocessing
$(OUTPREFIX)/%.i: $(KERNPREFIX)/%.c
	@mkdir -p $(OUTPREFIX)
	$(CC) $(CFLAGS) -E -o $@ $<

# kernel object files
$(KOBJPREFIX)/%.o: $(KERNPREFIX)/%.c
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJPREFIX)/%.o: $(KLIBPREFIX)/%.c
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJPREFIX)/%.o: $(KAPPPREFIX)/%.c
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KOBJPREFIX)/%.o: $(KERNPREFIX)/%.S
	@mkdir -p $(KOBJPREFIX)
	$(CC) $(ASFLAGS) -c -o $@ $<

# user object files
$(UOBJPREFIX)/%.o: $(UAPPPREFIX)/%.c
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(UOBJPREFIX)/%.o: $(ULIBPREFIX)/%.c
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

$(UOBJPREFIX)/%.o: $(ULIBPREFIX)/%.S
	@mkdir -p $(UOBJPREFIX)
	$(CC) $(CFLAGS) -c -o $@ $<

# misc generated
## vectors
$(KERNPREFIX)/vectors.S: $(TOOLSPREFIX)/vectors.py
	python3 $< > $(KERNPREFIX)/vectors.S

## bootblock: boot code for BSP
$(OUTPREFIX)/bootblock: $(BOOTPREFIX)/bootasm.S $(BOOTPREFIX)/bootmain.c
	@mkdir -p $(BOOTOBJPREFIX)
	# optimize for size here so bootblock fits in 510 bytes
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -Os -o $(BOOTOBJPREFIX)/bootmain.o -c $(BOOTPREFIX)/bootmain.c
	$(CC) -fno-builtin -fno-pic -m32 -nostdinc -I$(INCLUDEPATH) -o $(BOOTOBJPREFIX)/bootasm.o -c $(BOOTPREFIX)/bootasm.S
	$(LD) -m elf_i386 -nodefaultlibs -N -e start -Ttext 0x7C00 -o $(BOOTOBJPREFIX)/bootblock.o $(BOOTOBJPREFIX)/bootasm.o $(BOOTOBJPREFIX)/bootmain.o
	$(OBJDUMP) -S $(BOOTOBJPREFIX)/bootblock.o > $(OUTPREFIX)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(BOOTOBJPREFIX)/bootblock.o $(OUTPREFIX)/bootblock
	python3 $(TOOLSPREFIX)/sign.py $(OUTPREFIX)/bootblock

## bootap: boot code for AP
$(OUTPREFIX)/bootap: $(BOOTPREFIX)/bootap.S
	@mkdir -p $(BOOTOBJPREFIX)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I$(INCLUDEPATH) -o $(BOOTOBJPREFIX)/bootap.o -c $<
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $(BOOTOBJPREFIX)/bootapl.o $(BOOTOBJPREFIX)/bootap.o
	$(OBJDUMP) -S $(BOOTOBJPREFIX)/bootapl.o > $(OUTPREFIX)/bootap.asm
	$(OBJCOPY) -S -O binary -j .text $(BOOTOBJPREFIX)/bootapl.o $(OUTPREFIX)/bootap

# kernel elf
ENTRYCODE = $(KOBJPREFIX)/entry64.o
LINKSCRIPT = $(KERNPREFIX)/kernel64.ld
BINARIES = $(OUTPREFIX)/bootap
$(KERNELELF): $(OBJS) $(ENTRYCODE) $(LINKSCRIPT) $(OUTPREFIX)/bootap
	$(LD) $(LDFLAGS) -T $(LINKSCRIPT) -o $(KERNELELF) $(ENTRYCODE) $(OBJS) -b binary $(BINARIES)
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

addr2line:
	@src/tools/decodepc

# QEMU and GDB ================================

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = -gdb tcp::$(GDBPORT)
CPUS ?= 4
QEMUOPTS = -net none -drive format=raw,file=$(XV6IMG) -smp $(CPUS) -m 16M -no-reboot $(QEMUEXTRA)

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


