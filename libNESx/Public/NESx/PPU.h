#ifndef NESX_PPU_H
#define NESX_PPU_H

#include <stdint.h>

typedef struct nesx nesx_t;

typedef struct nesx_ppu
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

    // Horizontal Position
    int HPos;

    // Vertical Position
    int VPos;

    uint8_t PatternTable[2][0x1000];

    uint8_t NameTable[4][0x400];

    uint8_t PaletteRAM[0x20];

} nesx_ppu_t;

void NESx_PPU_Init(nesx_t * ctx);

void NESx_PPU_Tick(nesx_t * ctx);

#endif // NESX_PPU_H