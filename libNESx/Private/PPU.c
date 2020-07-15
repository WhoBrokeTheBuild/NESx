#include <NESx/PPU.h>
#include <NESx/NESx.h>

#include <string.h>

void NESx_PPU_Init(nesx_t * ctx)
{
    nesx_ppu_t * ppu = &ctx->PPU;

    memset(ppu->VRAM, 0, sizeof(ppu->VRAM));
    memset(ppu->PaletteRAM, 0, sizeof(ppu->PaletteRAM));

    ppu->PatternTables = NULL;
    memset(ppu->NameTables, 0, sizeof(ppu->NameTables));
}

void NESx_PPU_Tick(nesx_t * ctx)
{
    nesx_ppu_t * ppu = &ctx->PPU;



    ++ppu->Cycle;
    if (ppu->Cycle == 340) {
        ppu->Cycle = 0;
        ++ppu->Scanline;
        if (ppu->Scanline == 261) {
            ppu->Scanline = 0;
        }
    }
}