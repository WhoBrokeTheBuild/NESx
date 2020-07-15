#include <NESx/NESx.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

    return true;
}

void NESx_Term(nesx_t * ctx)
{
    NESx_ROM_Term(ctx);
    NESx_MMU_Term(ctx);
}

void NESx_Tick(nesx_t * ctx)
{
    NESx_MMU_CPU_Tick(ctx);
    MOS6502_Tick(&ctx->CPU);

    for (int i = 0; i < 3; ++i) {
        NESx_MMU_PPU_Tick(ctx);
        NESx_PPU_Tick(ctx);
    }
}