#ifndef NESX_H
#define NESX_H

#include <stdbool.h>
#include <stddef.h>

#include <NESx/MOS6502/MOS6502.h>
#include <NESx/ROM.h>

typedef struct nesx
{
    mos6502_t CPU;

    nesx_rom_header_t ROMHeader;

    uint8_t InternalRAM[0x800];

    uint8_t * ROM;
    size_t ROMSize;

} nesx_t;

bool NESx_Init(nesx_t * ctx);

void NESx_Term(nesx_t * ctx);

void NESx_Tick(nesx_t * ctx);

#endif // NESX_H