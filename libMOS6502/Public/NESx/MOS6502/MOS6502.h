#ifndef NESX_MOS6502_H
#define NESX_MOS6502_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mos6502
{
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

    // Status Register
    bool FC; // Carry
    bool FZ; // Zero
    bool FI; // Interrupt
    bool FD; // Decimal
    bool FV; // Overflow
    bool FN; // Negative

    // Output Pins
    uint8_t RW;
    uint8_t SYNC; // New Instruction Started

    // Input Pins
    uint8_t IRQ; // Maskable Interrupt Requested
    uint8_t NMI; // Non-Maskable Interrupt Requested
    uint8_t RDY; // "freeze execution at next read cycle"
    uint8_t RES; // Reset Requested

    uint64_t Cycles;

    bool BCDEnabled;

} mos6502_t;

void MOS6502_Init(mos6502_t * cpu);

void MOS6502_Tick(mos6502_t * cpu);

void MOS6502_SetStatusRegister(mos6502_t * cpu, uint8_t p);

uint8_t MOS6502_GetStatusRegister(mos6502_t * cpu);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NESX_MOS6502_H