/*
 * This should be the sole file included by all rt64 user c sources.
 */
#pragma once

#include "syscallconsts.h"

#define NULL ((void *)0)

int usysprint(const char *p);
/*
 * Return generic void* than a int.
 * Users can just use `usysexit((void*) 3)`, as long as they do not dereference the retval.
 */
void usysexit(void *retval) __attribute__((noreturn));
