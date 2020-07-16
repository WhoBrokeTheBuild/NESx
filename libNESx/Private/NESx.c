#include <NESx/NESx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool NESx_Init(nesx_t * ctx)
{
    MOS6502_Init(&ctx->CPU);
    ctx->CPU.BCDEnabled = false;

    // http://wiki.nesdev.com/w/index.php/CPU_power_up_state

    MOS6502_SetStatusRegister(&ctx->CPU, 0x34);
    ctx->CPU.A = 0;
    ctx->CPU.X = 0;
    ctx->CPU.Y = 0;
    ctx->CPU.S = 0xFD;

    NESx_MMU_Init(ctx);
    NESx_PPU_Init(ctx);

    return true;
}

void NESx_Term(nesx_t * ctx)
{
    NESx_ROM_Term(ctx);
    NESx_MMU_Term(ctx);
}

void NESx_Tick(nesx_t * ctx)
{
    // 1 CPU Tick
    NESx_MMU_CPU_Tick(ctx);
    MOS6502_Tick(&ctx->CPU);

    // 3 PPU Ticks
    NESx_MMU_PPU_Tick(ctx);
    NESx_PPU_Tick(ctx);

    NESx_MMU_PPU_Tick(ctx);
    NESx_PPU_Tick(ctx);

    NESx_MMU_PPU_Tick(ctx);
    NESx_PPU_Tick(ctx);
}

// clang-format off

void NESx_Step(nesx_t * ctx)
{
    do {
        NESx_Tick(ctx);
    }
    while (!ctx->CPU.SYNC);
}

void NESx_Frame(nesx_t * ctx)
{
    int scanline;
    do {
        scanline = ctx->PPU.Scanline;
        NESx_Tick(ctx);
    }
    while (scanline <= ctx->PPU.Scanline);
}

// clang-format on