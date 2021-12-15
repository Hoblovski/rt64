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

static inline i64 xchg64(volatile i64 *addr, i64 newval)
{
	i64 result;

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

// As in SDM2, RDTSC is not serializing instruction.
// When used to test very short time intervals, insert a serializing instr e.g. CPUID.
static inline u64 rdtsc()
{
	u64 eax, edx;
	__asm__ volatile("rdtsc" : "=a"(eax), "=d"(edx));
	return (edx << 32) | eax;
}

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
