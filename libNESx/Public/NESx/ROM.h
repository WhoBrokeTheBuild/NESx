#ifndef NESX_ROM_H
#define NESX_ROM_H

#include <stdbool.h>
#include <stdint.h>

#define NESX_TRAINER_SIZE      0x200
#define NESX_PRG_ROM_BANK_SIZE 0x4000
#define NESX_PRG_RAM_BANK_SIZE 0x2000
#define NESX_CHR_ROM_BANK_SIZE 0x2000
// #define NESX_CHR_RAM_BANK_SIZE 0x2000

typedef struct nesx nesx_t;

typedef struct nesx_rom_header
{
    union
    {
        struct
        {
            uint8_t Magic[4];

            uint8_t PRGROMBanks; // * 16kB
            uint8_t CHRROMBanks; // * 8kB

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

            uint8_t PRGRAMBanks; // * 8kB

            uint8_t Flags9;
            uint8_t Flags10;

            uint8_t _[5];
        };

        uint8_t raw[16];
    };

    uint8_t MapperNumber;

} nesx_rom_header_t;

typedef struct nesx_rom
{
    nesx_rom_header_t Header;

    uint8_t * Trainer;

    uint8_t * PRGROM;
    size_t PRGROMSize;

    uint8_t * PRGRAM;
    size_t PRGRAMSize;

    uint8_t * CHRROM;
    size_t CHRROMSize;

} nesx_rom_t;

bool NESx_ROM_Load(nesx_t * ctx, const char * filename);

void NESx_ROM_Term(nesx_t * ctx);

void NESx_ROM_PrintHeader(nesx_t * ctx);

const char * NESx_GetMapperName(nesx_t * ctx);

#endif // NESX_ROM_H