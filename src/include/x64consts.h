#pragma once
// X64 platform encodings

// flags register
#define FL_CF 0x00000001 // Carry Flag
#define FL_PF 0x00000004 // Parity Flag
#define FL_AF 0x00000010 // Auxiliary carry Flag
#define FL_ZF 0x00000040 // Zero Flag
#define FL_SF 0x00000080 // Sign Flag
#define FL_TF 0x00000100 // Trap Flag
#define FL_IF 0x00000200 // Interrupt Enable
#define FL_DF 0x00000400 // Direction Flag
#define FL_OF 0x00000800 // Overflow Flag
#define FL_IOPL_MASK 0x00003000 // I/O Privilege Level bitmask
#define FL_IOPL_0 0x00000000 //   IOPL == 0
#define FL_IOPL_1 0x00001000 //   IOPL == 1
#define FL_IOPL_2 0x00002000 //   IOPL == 2
#define FL_IOPL_3 0x00003000 //   IOPL == 3
#define FL_NT 0x00004000 // Nested Task
#define FL_RF 0x00010000 // Resume Flag
#define FL_VM 0x00020000 // Virtual 8086 mode
#define FL_AC 0x00040000 // Alignment Check
#define FL_VIF 0x00080000 // Virtual Interrupt Flag
#define FL_VIP 0x00100000 // Virtual Interrupt Pending
#define FL_ID 0x00200000 // ID flag

// Control Register flags
#define CR0_PE 0x00000001 // Protection Enable
#define CR0_MP 0x00000002 // Monitor coProcessor
#define CR0_EM 0x00000004 // Emulation
#define CR0_TS 0x00000008 // Task Switched
#define CR0_ET 0x00000010 // Extension Type
#define CR0_NE 0x00000020 // Numeric Errror
#define CR0_WP 0x00010000 // Write Protect
#define CR0_AM 0x00040000 // Alignment Mask
#define CR0_NW 0x20000000 // Not Writethrough
#define CR0_CD 0x40000000 // Cache Disable
#define CR0_PG 0x80000000 // Paging
#define CR4_PSE 0x00000010 // Page size extension

// Segment selectors
#define SEG_KCODE 1 // kernel code
#define SEG_KDATA 2 // kernel data+stack
#define SEG_KCPU 3 // kernel per-cpu data
#define SEG_UCODE 4 // user code
#define SEG_UDATA 5 // user data+stack
#define SEG_TSS 6 // this process's task state

#define DPL_USER 0x3 // User DPL

// Application segment type bits
#define STA_X 0x8 // Executable segment
#define STA_E 0x4 // Expand down (non-executable segments)
#define STA_C 0x4 // Conforming code segment (executable only)
#define STA_W 0x2 // Writeable (non-executable segments)
#define STA_R 0x2 // Readable (executable segments)
#define STA_A 0x1 // Accessed

// System segment type bits
#define STS_T16A 0x1 // Available 16-bit TSS
#define STS_LDT 0x2 // Local Descriptor Table
#define STS_T16B 0x3 // Busy 16-bit TSS
#define STS_CG16 0x4 // 16-bit Call Gate
#define STS_TG 0x5 // Task Gate / Coum Transmitions
#define STS_IG16 0x6 // 16-bit Interrupt Gate
#define STS_TG16 0x7 // 16-bit Trap Gate
#define STS_T32A 0x9 // Available 32-bit TSS
#define STS_T32B 0xB // Busy 32-bit TSS
#define STS_CG32 0xC // 32-bit Call Gate
#define STS_IG32 0xE // 32-bit Interrupt Gate
#define STS_TG32 0xF // 32-bit Trap Gate

// Paging
#define PG_SZ4K 0x1000 // Size of a 4K page in bytes
#define PG_SZ2M 0x200000 // Size of a 2M huge page in bytes
#define PG_SZ1G 0x40000000 // Size of a 1G huge page in bytes
#define PG_NENT 512 // Number of entries in a page table

// Shifts
#define PT_SHIFT 12
#define PD_SHIFT 21
#define PDPT_SHIFT 30
#define PML4_SHIFT 39
#define PML4_IDX(va) (((va) >> PML4_SHIFT) & (PG_NENT - 1))
#define PDPT_IDX(va) (((va) >> PDPT_SHIFT) & (PG_NENT - 1))
#define PD_IDX(va) (((va) >> PD_SHIFT) & (PG_NENT - 1))
#define PT_IDX(va) (((va) >> PT_SHIFT) & (PG_NENT - 1))
#define PE_ADDR(ent) ((ent) & (~((1 << 12) - 1)))
#define PE_FLAGS(ent) ((ent) & ((1 << 12) - 1))

// Page table flags
#define PF_P 1 // Present
#define PF_W 2 // Writable
#define PF_WP 3 // Writable and present
#define PF_U 4 // User accessible
#define PF_PWT 8 // Write through
#define PF_PCD 16 // Cache disable
#define PF_PS 128 // Huge page

// x86 trap and interrupt constants.
#define T_DIVIDE 0 // divide error
#define T_DEBUG 1 // debug exception
#define T_NMI 2 // non-maskable interrupt
#define T_BRKPT 3 // breakpoint
#define T_OFLOW 4 // overflow
#define T_BOUND 5 // bounds check
#define T_ILLOP 6 // illegal opcode
#define T_DEVICE 7 // device not available
#define T_DBLFLT 8 // double fault
// #define T_COPROC      9      // reserved (not used since 486)
#define T_TSS 10 // invalid task switch segment
#define T_SEGNP 11 // segment not present
#define T_STACK 12 // stack exception
#define T_GPFLT 13 // general protection fault
#define T_PGFLT 14 // page fault
// #define T_RES        15      // reserved
#define T_FPERR 16 // floating point error
#define T_ALIGN 17 // aligment check
#define T_MCHK 18 // machine check
#define T_SIMDERR 19 // SIMD floating point error

// These are arbitrarily chosen, but with care not to overlap
// processor defined exceptions or interrupt vectors.
#define T_SYSCALL 64 // system call
#define T_DEFAULT 500 // catchall

#define T_IRQ0 32 // IRQ 0 corresponds to int T_IRQ

#define IRQ_TIMER 0
#define IRQ_KBD 1
#define IRQ_COM1 4
#define IRQ_IDE 14
#define IRQ_ERROR 19
#define IRQ_SPURIOUS 31

// LAPIC and IOAPIC
#define DEFAULT_LAPIC_PADDR 0xFEE00000
#define DEFAULT_IOAPIC_PADDR 0xFEC00000
