#include <NESx/PPU.h>
#include <NESx/NESx.h>

#include <string.h>

void NESx_PPU_Init(nesx_t * ctx)
{
    nesx_ppu_t * ppu = &ctx->PPU;

    memset(ppu->PatternTable, 0, sizeof(ppu->PatternTable));
    memset(ppu->NameTable, 0, sizeof(ppu->NameTable));
    memset(ppu->PaletteRAM, 0, sizeof(ppu->PaletteRAM));
}

void NESx_PPU_Tick(nesx_t * ctx)
{
    // nesx_ppu_t * ppu = &ctx->PPU;
    
    // TODO
}