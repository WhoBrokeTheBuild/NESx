#ifndef MOS6502_H
#define MOS6502_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <MOS6502/Macros.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mos6502;

typedef void (*mos6502_instruction_t)(struct mos6502 *);

typedef enum mos6502_state
{
    MOS6502_STATE_IDLE = 0,
    MOS6502_STATE_FETCH,
    MOS6502_STATE_FETCH_DB,
    MOS6502_STATE_FETCH_ADL,
    MOS6502_STATE_FETCH_ADH,
    MOS6502_STATE_LOAD_DB,
    MOS6502_STATE_LOAD_ADL,
    MOS6502_STATE_LOAD_ADH,
    MOS6502_STATE_STORE,

} mos6502_state_t;

static const char * mos6502_state_str(mos6502_state_t state)
{
    static const char * STATE_STR[] = {
        "IDLE",
        "FETCH (IR)",
        "FETCH (DB)",
        "FETCH (ADL)",
        "FETCH (ADH)",
        "LOAD (DB)",
        "LOAD (ADL)",
        "LOAD (ADH)",
        "STORE",
    };
    return STATE_STR[(int)state];
}

typedef struct mos6502_instruction_cycle
{
    mos6502_instruction_t Instruction;
    mos6502_state_t NewState;

} mos6502_instruction_cycle_t;

// clang-format off

MOS6502_PACK(union mos6502_flags {
    struct
    {
        uint8_t N : 1; // Negative
        uint8_t V : 1; // Overflow
        uint8_t   : 1; // Unused
        uint8_t B : 1; // Break
        uint8_t D : 1; // Decimal
        uint8_t I : 1; // Interrupt
        uint8_t Z : 1; // Zero
        uint8_t C : 1; // Carry
    };

    uint8_t raw;
});

// clang-format on

typedef union mos6502_flags mos6502_flags_t;

static_assert(sizeof(mos6502_flags_t) == 1, "sizeof(mos6502_flags_t) != 1");

typedef enum mos6502_rw
{
    MOS6502_RW_WRITE = 0,
    MOS6502_RW_READ = 1,

} mos6502_rw_t;

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
    mos6502_rw_t RW;
    bool SYNC; // New Instruction Started

    // Input Pins
    bool IRQ; // Maskable Interrupt Requested
    bool NMI; // Non-Maskable Interrupt Requested
    bool RDY; // "freeze execution at next read cycle"
    bool RES; // Reset Requested

    bool BCDEnabled;

} mos6502_t;

void mos6502_init(mos6502_t * cpu);

void mos6502_tick(mos6502_t * cpu);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MOS6502_H