#ifndef WHL_H
#define WHL_H 1

// Planned API
// function window 

#ifndef whl_function
# define whl_function
#endif

#if defined (__x86_64__) || defined (__x86_64) || defined (__amd64) || defined (__amd64__)
# define WHL_ARCH_AMD64 1
#else
# error WHL: Unsupported architecture. Only amd64(x86_64) supported.
#endif

#ifndef WHL_INTEGER_TYPES
# define WHL_INTEGER_TYPES 1

typedef unsigned char  whl_u8;
typedef unsigned short whl_u16;
typedef unsigned int   whl_u32;
typedef unsigned long  whl_u64;

typedef signed char  whl_i8;
typedef signed short whl_i16;
typedef signed int   whl_i32;
typedef signed long  whl_i64;

typedef whl_u8  whl_b8;
typedef whl_u16 whl_b16;
typedef whl_u32 whl_b32;
typedef whl_b8  whl_bool;

typedef float  whl_f32;
typedef double whl_f64;

#ifdef WHL_ARCH_AMD64

typedef whl_i64 whl_int;
typedef whl_u64 whl_uint;

#else
# error WHL: Could not set register sized types.
#endif



#endif

#endif

// For editor
// TODO(robin): remove
#define WHL_IMPLEMENTATION

#ifdef WHL_IMPLEMENTATION

#ifndef whl_function_internal
# define whl_function_internal static
#endif

#endif
