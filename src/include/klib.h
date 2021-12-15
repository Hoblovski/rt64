#pragma once

// ======================================== consts
#define NULL ((void *)0)

// ======================================== string.c
void *memset(void *dst, int c, usize n);
int memcmp(const void *v1, const void *v2, usize n);
void *memmove(void *dst, const void *src, usize n);
void *memcpy(void *dst, const void *src, usize n);
int strncmp(const char *p, const char *q, usize n);
char *strncpy(char *s, const char *t, int n);
char *safestrcpy(char *s, const char *t, int n);
int strlen(const char *s);

// ======================================== atomic.h
// copied from linux

#define __always_inline inline __attribute__((__always_inline__))
#define LOCK_PREFIX "\n\tlock; "

typedef struct {
	i64 counter;
} atomic64_t;

static __always_inline void arch_atomic64_add(i64 i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
		     : "=m"(v->counter)
		     : "er"(i), "m"(v->counter)
		     : "memory");
}

static __always_inline void arch_atomic64_inc(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "incq %0"
		     : "=m"(v->counter)
		     : "m"(v->counter)
		     : "memory");
}

static __always_inline void arch_atomic64_dec(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "decq %0"
		     : "=m"(v->counter)
		     : "m"(v->counter)
		     : "memory");
}

static __always_inline i64 arch_atomic64_fetch_add(i64 i, atomic64_t *v)
{
	i64 ret = i;
	asm volatile("\n\txaddq %q0, %1"
		     : "+r"(ret), "+m"(v->counter)
		     :
		     : "memory", "cc");
	return ret;
}
