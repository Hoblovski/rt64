#pragma once

typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef long i64;
typedef unsigned long u64;
typedef long isize;
typedef unsigned long usize;

STATIC_ASSERT(sizeof(i16) == 2, bad_platform);
STATIC_ASSERT(sizeof(u16) == 2, bad_platform);
STATIC_ASSERT(sizeof(i32) == 4, bad_platform);
STATIC_ASSERT(sizeof(u32) == 4, bad_platform);
STATIC_ASSERT(sizeof(i64) == 8, bad_platform);
STATIC_ASSERT(sizeof(u64) == 8, bad_platform);
STATIC_ASSERT(sizeof(isize) == XLENB, bad_platform);
STATIC_ASSERT(sizeof(usize) == XLENB, bad_platform);
