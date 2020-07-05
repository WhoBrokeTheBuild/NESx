#ifndef NESX_ROM_H
#define NESX_ROM_H

#include <stdbool.h>
#include <stdint.h>

struct nesx;
typedef struct nesx nesx_t;

typedef struct nesx_rom_header
{
    union
    {
        struct
        {
            uint8_t Magic[4];

            uint8_t PRGROMSize; // * 16KB
            uint8_t CHRROMSize; // * 8KB, 0 = CHR RAM

            // Flags6
            struct
            {
                // Mirroring Type (0: horizontal/mapper, 1: vertical)
                uint8_t MirrorType : 1;
                // Battery/non-volatile memory (0: no, 1: yes)
                uint8_t Battery : 1;
                // 512 Byte Trainer  (0: no, 1: yes)
                uint8_t Trainer : 1;
                uint8_t FourScreenMode : 1;
                uint8_t MapperLow : 4;
            };

            // Flags7
            struct
            {
                // Console Type (0: NES, 1: Vs., 2: Playchoice, 3: Extended)
                uint8_t ConsoleType : 2;
                // NES 2.0 identifier: 2
                uint8_t ROMVersion : 2;
                uint8_t MapperHigh : 4;
            };

            uint8_t Flags8;
            uint8_t Flags9;
            uint8_t Flags10;

            uint8_t _[5];
        };

        uint8_t raw[16];
    };

    uint8_t MapperNumber;

} nesx_rom_header_t;

bool NESx_LoadROM(nesx_t * ctx, const char * filename);

void NESx_PrintROMHeader(nesx_t * ctx);

#endif // NESX_ROM_H