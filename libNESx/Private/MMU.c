#include <NESx/MMU.h>
#include <NESx/NESx.h>

#include <stdlib.h>
#include <string.h>

// http://wiki.nesdev.com/w/index.php/CPU_memory_map

void NESx_MMU_Init(nesx_t * ctx)
{
    nesx_mmu_t * mmu = &ctx->MMU;

    memset(mmu->InternalRAM, 0, sizeof(mmu->InternalRAM));

    mmu->Mapper = NULL;
}

void NESx_MMU_Term(nesx_t * ctx)
{
    nesx_mmu_t * mmu = &ctx->MMU;

    free(mmu->Mapper);
    mmu->Mapper = NULL;
}

uint8_t NESx_MMU_CPU_ReadByte(nesx_t * ctx, uint16_t address)
{
    nesx_mmu_t * mmu = &ctx->MMU;
    nesx_ppu_t * ppu = &ctx->PPU;

    switch (address >> 12) {
    case 0x0:
    case 0x1:
        return mmu->InternalRAM[address % sizeof(mmu->InternalRAM)];
        break;
    case 0x2:
    case 0x3:
        switch (address) {
        case 0x2002:
            return ppu->Status;
        }
        break;
    case 0x4:
        if (address <= 0x4017) {
            // APU, I/O
        }
        else {
            // CPU Test Mode
            // https://wiki.nesdev.com/w/index.php/CPU_Test_Mode
            assert(false);
            break;
        }
    case 0x5:
    case 0x6:
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF: return mmu->Mapper->PRGReadByte(ctx, address);
    }

    return 0x00;
}

uint8_t NESx_MMU_PPU_ReadByte(nesx_t * ctx, uint16_t address)
{
    nesx_ppu_t * ppu = &ctx->PPU;
    nesx_mmu_t * mmu = &ctx->MMU;

    uint16_t index;

    switch (address >> 12) {
    case 0x0:
    case 0x1: return mmu->Mapper->CHRReadByte(ctx, address);
    case 0x2:
    case 0x3:
        if (address >= 0x3F00) {
            return ppu->PaletteRAM[(address - 0x3F00) % sizeof(ppu->PaletteRAM)];
        }

        address -= 0x2000;
        address %= 0x1000;
        return ppu->NameTables[address / 0x400][address % 0x400];
    default: assert(false);
    };

    return 0x00;
}

void NESx_MMU_CPU_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data)
{
    nesx_mmu_t * mmu = &ctx->MMU;

    switch (address >> 12) {
    case 0x0:
    case 0x1:
        mmu->InternalRAM[address % sizeof(mmu->InternalRAM)] = data;
        break;
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

void NESx_MMU_PPU_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data)
{
    // TODO:
}

void NESx_MMU_CPU_Tick(nesx_t * ctx)
{
    mos6502_t * cpu = &ctx->CPU;

    if (cpu->RW) {
        cpu->DB = NESx_MMU_CPU_ReadByte(ctx, cpu->AB);
    }
    else {
        NESx_MMU_CPU_WriteByte(ctx, cpu->AB, cpu->DB);
    }
    cpu->RDY = true;
}

void NESx_MMU_PPU_Tick(nesx_t * ctx)
{
    nesx_ppu_t * ppu = &ctx->PPU;

    ppu->DB = NESx_MMU_CPU_ReadByte(ctx, ppu->AB);
}
