#include <NESx/Macros.h>
#include <NESx/NESx.h>
#include <NESx/ROM.h>

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

bool NESx_LoadROM(nesx_t * ctx, const char * filename)
{
    nesx_rom_header_t * hdr = &ctx->ROMHeader;

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

    if (hdr->Trainer) {
        fseek(fp, 0x200, SEEK_CUR);
    }

    ctx->ROMSize = hdr->PRGROMSize * 0x4000;
    ctx->ROM = (uint8_t *)malloc(ctx->ROMSize);

    bytesRead = fread(ctx->ROM, 1, ctx->ROMSize, fp);
    if (bytesRead < ctx->ROMSize) {
        fprintf(stderr, "failed to read ROM");
        return false;
    }

    hdr->MapperNumber = (hdr->MapperHigh << 4) | (hdr->MapperLow);
    switch (hdr->MapperNumber) {
    // NROM
    case 0: break;

    default:
        fprintf(stderr, "unsupported mapper #%d\n", hdr->MapperNumber);
        return false;
    };

    ctx->CPU.PCL = NESx_MMU_CPU_ReadByte(ctx, 0xFFFE);
    ctx->CPU.PCH = NESx_MMU_CPU_ReadByte(ctx, 0xFFFF);

    return true;
}

void NESx_PrintROMHeader(nesx_t * ctx)
{
    nesx_rom_header_t * hdr = &ctx->ROMHeader;

    printf("%s, Mapper: #%d, PRG: %dx16kB, CHR: %dx8kB, %s\n",
        (hdr->ROMVersion == 2 ? "NES 2.0" : "iNES"),
        hdr->MapperNumber,
        hdr->PRGROMSize,
        hdr->CHRROMSize,
        (hdr->MirrorType ? "V-Mirror" : "H-Mirror"));
}

const char * NESx_GetMapperName(nesx_t * ctx)
{
    switch (ctx->ROMHeader.MapperNumber) {
    case 0:     return "NROM";
    case 1:     return "MMC1";
    case 2:     return "UxROM";
    case 3:     return "CNROM";
    case 4:     return "MMC3";
    case 5:     return "MMC5";
    default:    return "Unsup.";
    }
}