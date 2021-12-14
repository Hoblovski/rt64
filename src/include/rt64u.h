/*
 * This should be the sole file included by all rt64 user c sources.
 */
#pragma once

#include "syscallconsts.h"

#define NULL ((void *)0)

int usysprint(const char *p);
void usysexit(void) __attribute__((noreturn));
