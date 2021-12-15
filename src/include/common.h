/*
 * Project independent header implementations.
 *
 * Basically augments compiler.
 */
#pragma once

#define STATIC_ASSERT(COND, MSG)                                               \
	static int static_assertion_##MSG[(COND) ? 1 : -1]                     \
		__attribute__((unused))

#define ASSERT(x)                                                              \
	do {                                                                   \
		if (!(x)) {                                                    \
			panic("assert: %s:%d (%s)\n", __FILE__, __LINE__, #x); \
		}                                                              \
	} while (0)
