#pragma once

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;
typedef long isize;
typedef unsigned long usize;
typedef u64 *pt_t;

STATIC_ASSERT(sizeof(i16) == 2, bad_platform);
STATIC_ASSERT(sizeof(u16) == 2, bad_platform);
STATIC_ASSERT(sizeof(i32) == 4, bad_platform);
STATIC_ASSERT(sizeof(u32) == 4, bad_platform);
STATIC_ASSERT(sizeof(i64) == 8, bad_platform);
STATIC_ASSERT(sizeof(u64) == 8, bad_platform);
STATIC_ASSERT(sizeof(isize) == XLENB, bad_platform);
STATIC_ASSERT(sizeof(usize) == XLENB, bad_platform);

// Saved registers for kernel context switches.
//	Specifically when `switch` function is called. (active switch)
//
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
//
// Don't need to save %rax,
// because it will contain the return value (and thus caller-saved).
//
// Don't need to save %r10, %r11, %rdi, %rsi, %rdx, %rcx, %r8, %r9,
// because they are caller-saved by x64 callingconv.
// Also don't need to save %rip, but see below.
//
// Contexts are stored at proc->context, and might be pointed to by percpu->idlectx.
//
// When calling `swtch`, %rip will be on top of the stack.
// But we still need this field, so we can designate a specific place to return to.
// Same for rdi.
struct context {
	u64 r15; // 0
	u64 r14; // 8
	u64 r13; // 16
	u64 r12; // 24
	u64 rbx; // 32
	u64 rbp; // 40
	u64 rip; // 48
	u64 rsp; // 56
	u64 rdi; // 64
};

// Saved registers for passive context switches i.e. traps.
struct trapframe {
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rbp;
	u64 rsi;
	u64 rdi;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;

	// below is saved by hardware and vector*
	u64 trapno;
	u64 err;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
};

// BELOW UNUSED
// XXX: this is not 64 bit.
// Task state segment format
struct taskstate {
	u32 link; // Old ts selector
	u32 esp0; // Stack pointers and segment selectors
	u16 ss0; //   after an increase in privilege level
	u16 padding1;
	u32 *esp1;
	u16 ss1;
	u16 padding2;
	u32 *esp2;
	u16 ss2;
	u16 padding3;
	u32 cr3; // Page directory base
	u32 *eip; // Saved state from last task switch
	u32 eflags;
	u32 eax; // More saved state (registers)
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 *esp;
	u32 *ebp;
	u32 esi;
	u32 edi;
	u16 es; // Even more saved state (segment selectors)
	u16 padding4;
	u16 cs;
	u16 padding5;
	u16 ss;
	u16 padding6;
	u16 ds;
	u16 padding7;
	u16 fs;
	u16 padding8;
	u16 gs;
	u16 padding9;
	u16 ldt;
	u16 padding10;
	u16 t; // Trap on task switch
	u16 iomb; // I/O map base address
};

// Gate descriptors for interrupts and traps
struct gatedesc {
	u32 off_15_0 : 16; // low 16 bits of offset in segment
	u32 cs : 16; // code segment selector
	u32 args : 5; // # args, 0 for interrupt/trap gates
	u32 rsv1 : 3; // reserved(should be zero I guess)
	u32 type : 4; // type(STS_{TG,IG32,TG32})
	u32 s : 1; // must be 0 (system)
	u32 dpl : 2; // descriptor(meaning new) privilege level
	u32 p : 1; // Present
	u32 off_31_16 : 16; // high bits of offset in segment
};

// Segment Descriptor
struct segdesc {
	u32 lim_15_0 : 16; // Low bits of segment limit
	u32 base_15_0 : 16; // Low bits of segment base address
	u32 base_23_16 : 8; // Middle bits of segment base address
	u32 type : 4; // Segment type (see STS_ constants)
	u32 s : 1; // 0 = system, 1 = application
	u32 dpl : 2; // Descriptor Privilege Level
	u32 p : 1; // Present
	u32 lim_19_16 : 4; // High bits of segment limit
	u32 avl : 1; // Unused (available for software use)
	u32 rsv1 : 1; // Reserved
	u32 db : 1; // 0 = 16-bit segment, 1 = 32-bit segment
	u32 g : 1; // Granularity: limit scaled by 4K when set
	u32 base_31_24 : 8; // High bits of segment base address
};

