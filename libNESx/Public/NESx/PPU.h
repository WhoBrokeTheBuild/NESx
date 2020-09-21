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

    // Vertical Position
    int Scanline;

    // Horizontal Position
    int Pixel;

    int Cycle;

    union
    {
        struct
        {
            uint8_t :5;
            uint8_t SpriteOverflow:1;
            uint8_t SpriteHit:1;
            uint8_t VBlank:1;
        };

        uint8_t Status;
    };

    // Video RAM
    uint8_t VRAM[0x800];

    // Will point to CHR ROM or whatever
    uint8_t * PatternTables;

    // Will point at VRAM by default
    uint8_t * NameTables[4];

    uint8_t PaletteRAM[0x20];

} nesx_ppu_t;

void NESx_PPU_Init(nesx_t * ctx);

void NESx_PPU_Tick(nesx_t * ctx);

#endif // NESX_PPU_H