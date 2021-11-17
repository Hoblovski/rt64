#pragma once
// included by rt64 kernel files

#define STATIC_ASSERT(COND, MSG)                                               \
	static int static_assertion_##MSG[(COND) ? 1 : -1]                     \
		__attribute__((unused))

#include "param.h" // #define's
#include "types.h" // typedefs and structs, extern variables
#include "defs.h" // function prototypes, extern variables
#include "mem.h" // memory layout, manipulation utilities
#include "x64.h" // platform specific

#include "klib.h"
