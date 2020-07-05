#include <NESx/MMU.h>
#include <NESx/NESx.h>

// http://wiki.nesdev.com/w/index.php/CPU_memory_map

uint8_t NESx_ReadByte(nesx_t * ctx, uint16_t address)
{
    switch (address >> 12) {
    case 0x0:
    case 0x1: return ctx->InternalRAM[address % sizeof(ctx->InternalRAM)];
    case 0x2:
    case 0x3:
        // PPU
        break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        // APU, I/O
        break;
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF: return ctx->ROM[(address - 0x8000) % ctx->ROMSize];
    }
    return 0x00;
}

void NESx_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data)
{
    // printf("Writing %02X to %04X\n", data, address);

    switch (address >> 12) {
    case 0x0:
    case 0x1: ctx->InternalRAM[address % sizeof(ctx->InternalRAM)] = data;
    case 0x2:
    case 0x3:
        // PPU
        break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        // APU, I/O
        break;
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
        break;
        // return ctx->ROM[(address - 0x8000) % ctx->ROMSize];
    }
}