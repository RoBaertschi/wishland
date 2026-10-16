#ifndef WHL_H
#define WHL_H 1

// Planned API
// function void whl_init(void);
// enum whl_flag { ... };
// function window *whl_window_create(WHL_S("Title"), whl_flag_...);

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

typedef whl_i64  whl_int;
typedef whl_u64  whl_uint;
typedef whl_uint whl_uintptr;

#else
# error WHL: Could not set register sized types.
#endif

struct Whl_String {
    whl_u8 const *data;
    whl_int      len;
};
typedef struct Whl_String whl_string;

#define WHL_S(sliteral) ((whl_string){ .data = (whl_u8 const *)(sliteral), (whl_int)sizeof(sliteral) - 1 })


whl_function void whl_init(void);
whl_function void whl_deinit(void);

#endif

#endif

// For editor
// TODO(robin): remove
#define WHL_IMPLEMENTATION

#ifdef WHL_IMPLEMENTATION

#ifndef whl_function_internal
# define whl_function_internal static
#endif

#ifndef whl_variable_internal
# define whl_variable_internal static
#endif

#ifndef WHL__GLUE
# define WHL__GLUE(a, b) WHL__GLUE_(a, b)
# define WHL__GLUE_(a, b) a##b
#endif

#ifndef WHL__MAX
# define WHL__MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef WHL__MIN
# define WHL__MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef WHL__CLAMP_TOP
# define WHL__CLAMP_TOP(value, top) WHL__MIN(value, top)
#endif

#ifndef WHL__CLAMP_BOT
# define WHL__CLAMP_BOT(bot, value) WHL__MAX(bot, value)
#endif

#ifndef WHL__TODO
# define WHL__TODO() (*((whl_u8 volatile*)0) = 0)
#endif

#ifndef WHL__KB
# define WHL__KB ((whl_int)(0) << 10)
#endif

#ifndef WHL__MB
# define WHL__MB ((whl_int)(0) << 20)
#endif

#ifndef WHL__GB
# define WHL__GB ((whl_int)(0) << 30)
#endif

#ifndef WHL__TB
# define WHL__TB ((whl_int)(0) << 40)
#endif

struct Whl__Wl_Display;
typedef struct Whl__Wl_Display Whl__Wl_Display;

struct Whl__Wl_Window;
typedef struct Whl__Wl_Window Whl__Wl_Window;

// X macro list for all the dynamically loaded wayland functions
// Format: X(name, ada_name, return type, ...args)
#define WHL__WL_FUNCTIONS \
    X(display_connect, Display_Connect, Whl__Wl_Display *, char const *name) \
    X(display_disconnect, Display_Disconnect, void, Whl__Wl_Display display)

#define X(_name, ada_name, return, ...) typedef return (*WHL__GLUE(Whl__Wl_Func_, ada_name))(__VA_ARGS__);
WHL__WL_FUNCTIONS
#undef X

struct Whl__System_Wl_Function_Table {
#define X(name, ada_name, ...) WHL__GLUE(Whl__Wl_Func_, ada_name) name;
WHL__WL_FUNCTIONS
#undef X
};
typedef struct Whl__System_Wl_Function_Table Whl__System_Wl_Function_Table;

struct Whl__System_Reserve_Result {
    void     *data;
    whl_bool ok;
};
typedef struct Whl__System_Reserve_Result Whl__System_Reserve_Result;

// This function reserve size bytes of memory. This should ideally use mmap with no read/write/execute rights.
// The size must be aligned up to the next page size, if not already aligned.
// The `data` in the return value must be 0 (NULL) when the allocation failed.
// If the allocation failed, `ok` in the return value must also be zero, else it must be a non-zero number.
// If `size` is 0, then `ok` must be true and `data` should be 0 (NULL).
whl_function_internal Whl__System_Reserve_Result whl__system_reserve(whl_uint size);

// This function commits the range of memory from `data` -> `size`.
// It should do that by setting the read/write permissions of that memory map.
// It must return 0 on failiure, and a non-zero value on success.
// Data must be aligned down to the next page size, if not already aligned.
// Size must be aligned up to the next page size, if not already aligned.
whl_function_internal whl_bool whl__system_commit(void *data, whl_uint size);

// This function release the range of memory from `data` -> `size`.
// It should do that by calling munmap on the provided range.
// This function cannot fail and will not report any sort of failiure in a program altering way.
// Data must be aligned down to the next page size, if not already aligned.
// Size must be aligned up to the next page size, if not already aligned.
whl_function_internal void whl__system_release(void *data, whl_uint size);

// Dynamically loads the system wayland library and returns a table of functions for it. Only gets called on init.
whl_function_internal Whl__System_Wl_Function_Table whl__system_load_wl(void);

// Unloads the previously loaded wayland library. Only gets called on deinit
whl_function_internal void whl__system_unload_wl(void);

#ifndef WHL_SYSTEM_IMPLEMENTATION
# define WHL_SYSTEM_FUNCTIONS
# define WHL_SYSTEM_PAGE_SIZE (whl_uint)4096

#include <sys/mman.h>
#include <dlfcn.h>

#define WHL__ALIGN_UP(value, align) (((value) + ((align) - 1)) & ~((align) - 1))

// This function reserve size bytes of memory. This should ideally use mmap with no read/write/execute rights.
// The size must be aligned up to the next page size, if not already aligned.
// The `data` in the return value must be 0 (NULL) when the allocation failed.
// If the allocation failed, `ok` in the return value must also be zero, else it must be a non-zero number.
// If `size` is 0, then `ok` must be true and `data` should be 0 (NULL).
whl_function_internal Whl__System_Reserve_Result whl__system_reserve(whl_uint size) {
    Whl__System_Reserve_Result result = { 0 };
    size = WHL__ALIGN_UP(size, WHL_SYSTEM_PAGE_SIZE);

    result.data = mmap(0, size, PROT_NONE, MAP_ANONYMOUS, -1, 0);
    result.ok   = result.data != MAP_FAILED;

    return result;
}

// This function commits the range of memory from `data` -> `size`.
// It should do that by setting the read/write permissions of that memory map.
// It must return 0 on failiure, and a non-zero value on success.
// Data must be aligned down to the next page size, if not already aligned.
// Size must be aligned up to the next page size, if not already aligned.
whl_function_internal whl_bool whl__system_commit(void *data, whl_uint size) {
    data = (void *)(WHL__ALIGN_UP((whl_uintptr)data, (whl_uintptr)WHL_SYSTEM_PAGE_SIZE));
    size = WHL__ALIGN_UP(size, WHL_SYSTEM_PAGE_SIZE);

    whl_bool ok = mprotect(data, size, PROT_READ | PROT_WRITE) == 0;
    return ok;
}

// This function release the range of memory from `data` -> `size`.
// It should do that by calling uunmap on the provided range.
// This function cannot fail and will not report any sort of failiure in a program altering way.
// Data must be aligned down to the next page size, if not already aligned.
// Size must be aligned up to the next page size, if not already aligned.
whl_function_internal void whl__system_release(void *data, whl_uint size) {
    data = (void *)(WHL__ALIGN_UP((whl_uintptr)data, (whl_uintptr)WHL_SYSTEM_PAGE_SIZE));
    size = WHL__ALIGN_UP(size, WHL_SYSTEM_PAGE_SIZE);

    munmap(data, size);
}

whl_function_internal void *whl__system_wl_library;

// Dynamically loads the system wayland library and returns a table of functions for it. Only gets called on init.
whl_function_internal Whl__System_Wl_Function_Table whl__system_load_wl(void) {
    Whl__System_Wl_Function_Table table = { 0 };

    whl__system_wl_library = dlopen("libwayland-client.so.0", RTLD_NOW);
    if (whl__system_wl_library == 0) {
        WHL__TODO(); // TODO(robin): support prober error handling
    }

    char *current_error = dlerror(); // clear dlerror
    current_error = 0; // Reset

#define X(name, _ada_name, ...) table.name    = dlsym(whl__system_wl_library, "wl_" #name); \
                                current_error = dlerror(); \
                                if (current_error != 0) {\
                                    WHL__TODO(); /* TODO(robin): support prober error handling */ \
                                }
    WHL__WL_FUNCTIONS
#undef X

    return table;
}

// Unloads the previously loaded wayland library. Only gets called on deinit
whl_function_internal void whl__system_unload_wl(void) {
    dlclose(whl__system_wl_library);
    whl__system_wl_library = 0;
}

#endif

#ifndef WHL_SYSTEM_PAGE_SIZE
# error WHL: WHL_WHL_SYSTEM_PAGE_SIZE not defined for custom system hooks implementation
#endif

struct Whl__Memory_Block {
    struct Whl__Memory_Block *prev;
    whl_u8   *data;
    whl_uint reserved;
    whl_uint commited;
    whl_uint used;
};
typedef struct Whl__Memory_Block Whl__Memory_Block;

whl_function_internal Whl__Memory_Block *whl__memory_block_new(whl_uint reserved, whl_uint commited) {
    commited = WHL__CLAMP_BOT(sizeof(Whl__Memory_Block), commited);
    reserved = WHL__CLAMP_BOT(sizeof(Whl__Memory_Block), reserved);
    reserved = WHL__CLAMP_BOT(commited, reserved);

    Whl__System_Reserve_Result reserve_result = whl__system_reserve(reserved);
    if (!reserve_result.ok) {
        WHL__TODO();
    }
    Whl__Memory_Block *result = reserve_result.data;
    result->data     = reserve_result.data;
    result->reserved = reserved;
    result->commited = commited;
    result->prev     = 0;
    result->used     = sizeof(Whl__Memory_Block);
    return result;
}

struct Whl__State {
    Whl__Memory_Block             *first, *last;
    Whl__System_Wl_Function_Table wl;
};
typedef struct Whl__State Whl__State;


whl_variable_internal Whl__State *state;

whl_function_internal whl_u8 *whl__alloc(whl_int size, whl_int alignment) {
    if (alignment <= 0) {
        alignment = 16;
    }

    whl_u8 *result = 0;

    if (0 < size) {
        Whl__Memory_Block *current = state->last;

        whl_uintptr unaligned_address = (whl_uintptr)current->data + (whl_uintptr)current->used;
        whl_uintptr aligned_address   = WHL__ALIGN_UP(unaligned_address, (whl_uintptr)alignment);
        whl_uintptr new_used          = aligned_address - (whl_uintptr)current->data;

        if (current->reserved < new_used + size) {
            // New block
            // A bit of over-allocation just to be sure
            Whl__Memory_Block *new_block = whl__memory_block_new(WHL__CLAMP_BOT(size + alignment + sizeof(Whl__Memory_Block), current->reserved),
                                                                 WHL__CLAMP_BOT(size + alignment + sizeof(Whl__Memory_Block), current->commited));

            new_block->prev = current;
            current         = new_block;
            state->last     = new_block;

            unaligned_address = (whl_uintptr)current->data + (whl_uintptr)current->used;
            aligned_address   = WHL__ALIGN_UP(unaligned_address, (whl_uintptr)alignment);
            new_used          = aligned_address - (whl_uintptr)current->data;
        }

        if (current->commited < new_used + size) {
            whl_uint new_commited = WHL__ALIGN_UP((new_used + size) - current->commited, WHL_SYSTEM_PAGE_SIZE);
            if (!whl__system_commit(current->data + current->commited, new_commited)) {
                WHL__TODO();
            }
            current->commited = new_commited;
        }

        current->used = new_used;

        result = current->data + new_used;
    }

    return result;
}

whl_function void whl_init(void) {
    Whl__Memory_Block *block = whl__memory_block_new(WHL__KB * 4 , WHL__MB * 128);
}
whl_function void whl_deinit(void);

#endif
