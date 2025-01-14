#pragma once


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;


#define STATIC_ASSERT _Static_assert

STATIC_ASSERT(sizeof(u8) == 1);
STATIC_ASSERT(sizeof(u16) == 2);
STATIC_ASSERT(sizeof(u32) == 4);
STATIC_ASSERT(sizeof(u64) == 8);

STATIC_ASSERT(sizeof(i8) == 1);
STATIC_ASSERT(sizeof(i16) == 2);
STATIC_ASSERT(sizeof(i32) == 4);
STATIC_ASSERT(sizeof(i64) == 8);

STATIC_ASSERT(sizeof(f32) == 4);
STATIC_ASSERT(sizeof(f64) == 8);
