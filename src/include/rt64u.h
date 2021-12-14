#pragma once

#include "syscall.h"

#define NULL ((void *)0)

int usysprint(const char *p);
void usysexit(void) __attribute__((noreturn));
