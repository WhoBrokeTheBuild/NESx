#ifndef NESX_MOS6502_H
#define NESX_MOS6502_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <NESx/MOS6502/Macros.h>

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off

MOS6502_PACK(union mos6502_flags
{
    struct
    {
        uint8_t C : 1; // Carry
        uint8_t Z : 1; // Zero
        uint8_t I : 1; // Interrupt
        uint8_t D : 1; // Decimal
        uint8_t B : 1; // Break
        uint8_t X : 1; // Unused
        uint8_t V : 1; // Overflow
        uint8_t N : 1; // Negative
    };

    uint8_t raw;
});

// clang-format on

typedef union mos6502_flags mos6502_flags_t;

static_assert(sizeof(mos6502_flags_t) == 1, "sizeof(mos6502_flags_t) != 1");

typedef struct mos6502
{
    /* Public */

    // Address Bus
    union
    {
        struct
        {
            uint8_t ABL;
            uint8_t ABH;
        };
        uint16_t AB;
    };

    // Data Bus
    uint8_t DB;

    /* Private */

    // Instruction Register (<< 3)
    uint16_t IR;

    // Program Counter
    union
    {
        struct
        {
            uint8_t PCL;
            uint8_t PCH;
        };
        uint16_t PC;
    };

    // Address Line
    union
    {
        struct
        {
            uint8_t ADL;
            uint8_t ADH;
        };
        uint16_t AD;
    };

    // Accumulator
    uint8_t A;

    // X Index
    uint8_t X;

    // Y Index
    uint8_t Y;

    // Stack Pointer
    uint8_t S;

    // Processor Flags
    mos6502_flags_t P;

    // Output Pins
    uint8_t RW;
    uint8_t SYNC; // New Instruction Started

    // Input Pins
    uint8_t IRQ; // Maskable Interrupt Requested
    uint8_t NMI; // Non-Maskable Interrupt Requested
    uint8_t RDY; // "freeze execution at next read cycle"
    uint8_t RES; // Reset Requested

    bool BCDEnabled;

} mos6502_t;

void MOS6502_Init(mos6502_t * cpu);

void MOS6502_Tick(mos6502_t * cpu);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NESX_MOS6502_H