#pragma once
// Routines to let C code use special x86 instructions.

static inline u8 inb(u16 port)
{
	u8 data;

	asm volatile("in %1,%0" : "=a"(data) : "d"(port));
	return data;
}

static inline void insl(int port, void *addr, int cnt)
{
	asm volatile("cld; rep insl"
		     : "=D"(addr), "=c"(cnt)
		     : "d"(port), "0"(addr), "1"(cnt)
		     : "memory", "cc");
}

static inline void outb(u16 port, u8 data)
{
	asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(u16 port, u16 data)
{
	asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outsl(int port, const void *addr, int cnt)
{
	asm volatile("cld; rep outsl"
		     : "=S"(addr), "=c"(cnt)
		     : "d"(port), "0"(addr), "1"(cnt)
		     : "cc");
}

static inline void stosb(void *addr, int data, int cnt)
{
	asm volatile("cld; rep stosb"
		     : "=D"(addr), "=c"(cnt)
		     : "0"(addr), "1"(cnt), "a"(data)
		     : "memory", "cc");
}

static inline void stosl(void *addr, int data, int cnt)
{
	asm volatile("cld; rep stosl"
		     : "=D"(addr), "=c"(cnt)
		     : "0"(addr), "1"(cnt), "a"(data)
		     : "memory", "cc");
}

struct segdesc;

static inline void lgdt(struct segdesc *p, int size)
{
	volatile u16 pd[5];

	pd[0] = size - 1;
	pd[1] = (usize)p;
	pd[2] = (usize)p >> 16;
	pd[3] = (usize)p >> 32;
	pd[4] = (usize)p >> 48;
	asm volatile("lgdt (%0)" : : "r"(pd));
}

struct gatedesc;

static inline void lidt(struct gatedesc *p, int size)
{
	volatile u16 pd[5];

	pd[0] = size - 1;
	pd[1] = (usize)p;
	pd[2] = (usize)p >> 16;
	pd[3] = (usize)p >> 32;
	pd[4] = (usize)p >> 48;
	asm volatile("lidt (%0)" : : "r"(pd));
}

static inline void ltr(u16 sel)
{
	asm volatile("ltr %0" : : "r"(sel));
}

static inline usize readeflags(void)
{
	usize eflags;
	asm volatile("pushf; pop %0" : "=r"(eflags));
	return eflags;
}

static inline void loadgs(u16 v)
{
	asm volatile("movw %0, %%gs" : : "r"(v));
}

static inline void cli(void)
{
	asm volatile("cli");
}

static inline void sti(void)
{
	asm volatile("sti");
}

static inline void hlt(void)
{
	asm volatile("hlt");
}

static inline u32 xchg(volatile u32 *addr, usize newval)
{
	u32 result;

	// The + in "+m" denotes a read-modify-write operand.
	asm volatile("lock; xchgl %0, %1"
		     : "+m"(*addr), "=a"(result)
		     : "1"(newval)
		     : "cc");
	return result;
}

static inline usize rcr2(void)
{
	usize val;
	asm volatile("mov %%cr2,%0" : "=r"(val));
	return val;
}

static inline void lcr3(usize val)
{
	asm volatile("mov %0,%%cr3" : : "r"(val));
}

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

	u64 trapno;
	u64 err;

	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
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

// Normal segment
#define SEG(type, base, lim, dpl)                                              \
	(struct segdesc)                                                       \
	{                                                                      \
		((lim) >> 12) & 0xffff, (u32)(base)&0xffff,                    \
			((usize)(base) >> 16) & 0xff, type, 1, dpl, 1,         \
			(usize)(lim) >> 28, 0, 0, 1, 1, (usize)(base) >> 24    \
	}
#define SEG16(type, base, lim, dpl)                                            \
	(struct segdesc)                                                       \
	{                                                                      \
		(lim) & 0xffff, (usize)(base)&0xffff,                          \
			((usize)(base) >> 16) & 0xff, type, 1, dpl, 1,         \
			(usize)(lim) >> 16, 0, 0, 1, 0, (usize)(base) >> 24    \
	}

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

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                                     \
	{                                                                      \
		(gate).off_15_0 = (u32)(off)&0xffff;                           \
		(gate).cs = (sel);                                             \
		(gate).args = 0;                                               \
		(gate).rsv1 = 0;                                               \
		(gate).type = (istrap) ? STS_TG32 : STS_IG32;                  \
		(gate).s = 0;                                                  \
		(gate).dpl = (d);                                              \
		(gate).p = 1;                                                  \
		(gate).off_31_16 = (u32)(off) >> 16;                           \
	}
