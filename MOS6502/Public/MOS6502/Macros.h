#ifndef MOS6502_MACROS_H
#define MOS6502_MACROS_H

#if defined(__MSC_VER)
#    define MOS6502_PACK(DECL) __pragma(pack(push, 1)) DECL __pragma(pack(pop))
#    define MOS6502_INLINE     __forceinline
#else
#    define MOS6502_PACK(DECL) DECL __attribute__((__packed__))
#    define MOS6502_INLINE     __attribute__((always_inline))
#endif

#endif // MOS6502_MACROS_H