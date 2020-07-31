#ifndef NESX_MAPPER_NROM_H
#define NESX_MAPPER_NROM_H

#include <NESx/MMU.h>
#include <NESx/NESx.h>

#include <stdlib.h>

static uint8_t NROM_PRGReadByte(nesx_t * ctx, uint16_t address)
{
    nesx_rom_t * rom = &ctx->ROM;

    // Assume address is >= 0x6000
    if (address <= 0x8000) {
        return rom->PRGRAM[(address - 0x6000) % rom->PRGRAMSize];
    }

    return rom->PRGROM[(address - 0x8000) % rom->PRGROMSize];
}

static void NROM_PRGWriteByte(nesx_t * ctx, uint16_t address, uint8_t data)
{
    nesx_rom_t * rom = &ctx->ROM;

    // Assume address is >= 0x6000
    if (address <= 0x8000) {
        rom->PRGRAM[(address - 0x6000) % rom->PRGRAMSize] = data;
    }
}

static nesx_mapper_t * NROM_New(nesx_t * ctx)
{
    nesx_mapper_t * map = (nesx_mapper_t *)malloc(sizeof(nesx_mapper_t));

    map->PRGReadByte = NROM_PRGReadByte;
    map->PRGWriteByte = NROM_PRGWriteByte;
    // map->CHRReadByte = NROM_CHRReadByte;
    // map->CHRWriteByte = NROM_CHRWriteByte;

    nesx_ppu_t * ppu = &ctx->PPU;

    ppu->PatternTables = ctx->ROM.CHRROM;

    if (ctx->ROM.Header.MirrorType) {
        ctx->PPU.NameTables[0] = ctx->PPU.VRAM;
        ctx->PPU.NameTables[1] = ctx->PPU.VRAM + 0x400;
        ctx->PPU.NameTables[2] = ctx->PPU.VRAM;
        ctx->PPU.NameTables[3] = ctx->PPU.VRAM + 0x400;
    }
    else {
        ctx->PPU.NameTables[0] = ctx->PPU.VRAM;
        ctx->PPU.NameTables[1] = ctx->PPU.VRAM;
        ctx->PPU.NameTables[2] = ctx->PPU.VRAM + 0x400;
        ctx->PPU.NameTables[3] = ctx->PPU.VRAM + 0x400;
    }

    return map;
}

#endif // NESX_MAPPER_NROM_H