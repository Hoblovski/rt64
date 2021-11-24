#pragma once

#include "types.h"
#include "x64.h"

// standard
#define NULL 0

// string.c
void *memset(void *dst, int c, usize n);
int memcmp(const void *v1, const void *v2, usize n);
void *memmove(void *dst, const void *src, usize n);
void *memcpy(void *dst, const void *src, usize n);
int strncmp(const char *p, const char *q, usize n);
char *strncpy(char *s, const char *t, int n);
char *safestrcpy(char *s, const char *t, int n);
int strlen(const char *s);
