#pragma once

typedef unsigned char u8;         // [0, 255]
typedef unsigned short u16;       // [0, 65.535]
typedef unsigned int u32;         // [0, 4.294.967.295]
typedef unsigned long long u64;   // [0, 18.446.744.073.709.551.615]

typedef signed char i8;           // [−127, +127]
typedef signed short i16;         // [−32767, +32767]
typedef signed int i32;           // [−2.147.483.647, +2.147.483.647]
typedef signed long long i64;     // [−9223372036854775807, +9223372036854775807] 

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
