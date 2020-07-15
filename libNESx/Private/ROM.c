#include <NESx/Macros.h>
#include <NESx/NESx.h>
#include <NESx/ROM.h>

#include "Mapper/NROM.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint8_t NES_ROM_MAGIC[] = {
    // NES<EOF>
    0x4E,
    0x45,
    0x53,
    0x1A,
};

bool NESx_ROM_Load(nesx_t * ctx, const char * filename)
{
    nesx_rom_t * rom = &ctx->ROM;
    nesx_rom_header_t * hdr = &rom->Header;

    FILE * fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "failed to open ROM '%s'\n", filename);
        return false;
    }

    size_t bytesRead = fread(hdr->raw, 1, sizeof(hdr->raw), fp);
    if (bytesRead < sizeof(hdr->raw)) {
        fprintf(stderr, "failed to read ROM header\n");
        return false;
    }

    if (memcmp(hdr->Magic, NES_ROM_MAGIC, sizeof(NES_ROM_MAGIC)) != 0) {
        fprintf(stderr, "invalid ROM magic\n");
        return false;
    }

    rom->Trainer = NULL;
    if (hdr->Trainer) {
        rom->Trainer = (uint8_t *)malloc(NESX_TRAINER_SIZE);
        bytesRead = fread(rom->Trainer, 1, NESX_TRAINER_SIZE, fp);
        if (bytesRead < NESX_TRAINER_SIZE) {
            fprintf(stderr, "failed to read Trainer\n");
        }
    }

    rom->PRGROMSize = hdr->PRGROMBanks * NESX_PRG_ROM_BANK_SIZE;
    rom->PRGROM = (uint8_t *)malloc(rom->PRGROMSize);
    assert(rom->PRGROM);

    bytesRead = fread(rom->PRGROM, 1, rom->PRGROMSize, fp);
    if (bytesRead < rom->PRGROMSize) {
        fprintf(stderr, "failed to read PRG ROM\n");
        return false;
    }

    rom->CHRROMSize = hdr->CHRROMBanks * NESX_CHR_ROM_BANK_SIZE;
    rom->CHRROM = (uint8_t *)malloc(rom->CHRROMSize);
    assert(rom->CHRROM);

    if (hdr->PRGRAMBanks == 0) {
        hdr->PRGRAMBanks = 1;
    }

    rom->PRGRAMSize = hdr->PRGRAMBanks * NESX_PRG_RAM_BANK_SIZE;
    rom->PRGRAM = (uint8_t *)malloc(rom->PRGRAMSize);
    assert(rom->PRGRAM);

    hdr->MapperNumber = (hdr->MapperHigh << 4) | (hdr->MapperLow);

    switch (hdr->MapperNumber) {
    case 0:
        ctx->MMU.Mapper = NROM_New(ctx);
        break;

    default:
        fprintf(stderr, "unsupported mapper #%d\n", hdr->MapperNumber);
        return false;
    };

    ctx->CPU.PCL = NESx_MMU_CPU_ReadByte(ctx, 0xFFFE);
    ctx->CPU.PCH = NESx_MMU_CPU_ReadByte(ctx, 0xFFFF);

    return true;
}

void NESx_ROM_Term(nesx_t * ctx)
{
    nesx_rom_t * rom = &ctx->ROM;

    free(rom->Trainer);
    rom->Trainer = NULL;

    free(rom->PRGROM);
    rom->PRGROM = NULL;
    
    free(rom->PRGRAM);
    rom->PRGRAM = NULL;
    
    free(rom->CHRROM);
    rom->CHRROM = NULL;
}

void NESx_ROM_PrintHeader(nesx_t * ctx)
{
    nesx_rom_header_t * hdr = &ctx->ROM.Header;

    printf("%s, Mapper: #%d, PRG: %d x 16kB, CHR: %d x 8kB, %s\n",
        (hdr->ROMVersion == 2 ? "NES 2.0" : "iNES"),
        hdr->MapperNumber,
        hdr->PRGROMBanks,
        hdr->CHRROMBanks,
        (hdr->MirrorType ? "V-Mirror" : "H-Mirror"));
}

const char * NESx_GetMapperName(nesx_t * ctx)
{
    switch (ctx->ROM.Header.MapperNumber) {
    case 0:     return "NROM";
    case 1:     return "MMC1";
    case 2:     return "UxROM";
    case 3:     return "CNROM";
    case 4:     return "MMC3";
    case 5:     return "MMC5";
    default:    return "Unsup.";
    }
}