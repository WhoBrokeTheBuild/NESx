#include <NESx/NESx.h>

#include <stdlib.h>
#include <string.h>

bool NESx_Init(nesx_t * ctx)
{
    MOS6502_Init(&ctx->CPU);
    ctx->CPU.BCDEnabled = false;

    // http://wiki.nesdev.com/w/index.php/CPU_power_up_state

    ctx->CPU.P.raw = 0x34;
    ctx->CPU.A = 0;
    ctx->CPU.X = 0;
    ctx->CPU.Y = 0;
    ctx->CPU.S = 0xFD;

    memset(&ctx->ROMHeader, 0, sizeof(ctx->ROMHeader));

    memset(ctx->InternalRAM, 0, sizeof(ctx->InternalRAM));

    ctx->ROM = NULL;
    ctx->ROMSize = 0;

    return true;
}

void NESx_Term(nesx_t * ctx)
{
    free(ctx->ROM);
    ctx->ROM = NULL;
    ctx->ROMSize = 0;

    // mos6502_Term(&ctx->CPU);
}