#pragma once
// included by rt64 kernel files

#define STATIC_ASSERT(COND, MSG)                                               \
	static int static_assertion_##MSG[(COND) ? 1 : -1]                     \
		__attribute__((unused))

#include "param.h" // #define's
#include "types.h" // typedefs and structs
#include "defs.h" // function prototypes
#include "x64.h" // platform specific
