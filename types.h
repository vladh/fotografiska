#pragma once

#if !defined(MAX_PATH)
#if defined(PATH_MAX)
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif
#endif

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int8 s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;

typedef int8 bool8;
typedef int16 bool16;
typedef int32 bool32;
typedef int64 bool64;
typedef bool8 b8;
typedef bool16 b16;
typedef bool32 b32;
typedef bool64 b64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef float real32;
typedef double real64;
typedef real32 f32;
typedef real64 f64;

#define KB_TO_B(Value) ((Value) * 1024LL)
#define MB_TO_B(Value) (KB_TO_B(Value) * 1024LL)
#define GB_TO_B(Value) (MB_TO_B(Value) * 1024LL)
#define TB_TO_B(Value) (GB_TO_B(Value) * 1024LL)

