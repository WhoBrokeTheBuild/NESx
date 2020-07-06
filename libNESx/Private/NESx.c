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

void NESx_Tick(nesx_t * ctx)
{
    if (ctx->CPU.RW) {
        ctx->CPU.DB = NESx_ReadByte(ctx, ctx->CPU.AB);
        ctx->CPU.RDY = true;
        // printf("Read %02X from %04X\n", ctx->CPU.DB, ctx->CPU.AB);
    }
    else {
        NESx_WriteByte(ctx, ctx->CPU.AB, ctx->CPU.DB);
        ctx->CPU.RDY = true;
        // printf("Wrote %02X to %04X\n", ctx->CPU.DB, ctx->CPU.AB);
    }

    // printf("IR:%02X:%d PC:%04X AB:%04X DB:%02X A:%02X X:%02X Y:%02X S:%02X P:%c%c%c%c%c%c\n",
    //     (ctx->CPU.IR >> 3), (ctx->CPU.IR & 0b111),
    //     ctx->CPU.PC, ctx->CPU.AB, ctx->CPU.DB, 
    //     ctx->CPU.A, ctx->CPU.X, ctx->CPU.Y, ctx->CPU.S,
    //     (ctx->CPU.FC ? 'C' : 'c'),
    //     (ctx->CPU.FZ ? 'Z' : 'z'),
    //     (ctx->CPU.FI ? 'I' : 'i'),
    //     (ctx->CPU.FD ? 'D' : 'd'),
    //     (ctx->CPU.FV ? 'V' : 'v'),
    //     (ctx->CPU.FN ? 'N' : 'n'));
    MOS6502_Tick(&ctx->CPU);
}