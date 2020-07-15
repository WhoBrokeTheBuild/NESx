#ifndef NESX_H
#define NESX_H

#include <stdbool.h>
#include <stddef.h>

#include <NESx/MOS6502/MOS6502.h>

#include <NESx/MMU.h>
#include <NESx/PPU.h>
#include <NESx/ROM.h>

typedef struct nesx
{
    mos6502_t CPU;

    nesx_ppu_t PPU;

    nesx_mmu_t MMU;

    nesx_rom_t ROM;

} nesx_t;

bool NESx_Init(nesx_t * ctx);

void NESx_Term(nesx_t * ctx);

void NESx_Tick(nesx_t * ctx);

#endif // NESX_H