#ifndef NESX_MACROS_H
#define NESX_MACROS_H

#if defined(__MSC_VER)
#    define NESX_PACK(DECL) __pragma(pack(push, 1)) DECL __pragma(pack(pop))
#    define NESX_INLINE     __forceinline
#else
#    define NESX_PACK(DECL) DECL __attribute__((__packed__))
#    define NESX_INLINE     __attribute__((always_inline))
#endif

#define NESX_UNUSED(V) (void)(V)

#endif // NESX_MACROS_H