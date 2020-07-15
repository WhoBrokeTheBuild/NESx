#ifndef NESX_MMU_H
#define NESX_MMU_H

#include <stdint.h>

typedef struct nesx nesx_t;

typedef uint8_t (*nesx_mapper_read_byte_t)(nesx_t *, uint16_t);
typedef void (*nesx_mapper_write_byte_t)(nesx_t *, uint16_t, uint8_t);

typedef struct nesx_mapper
{
    nesx_mapper_read_byte_t PRGReadByte;
    nesx_mapper_write_byte_t PRGWriteByte;

    nesx_mapper_read_byte_t CHRReadByte;
    nesx_mapper_write_byte_t CHRWriteByte;

} nesx_mapper_t;

typedef struct nesx_mmu
{
    uint8_t InternalRAM[0x800];

    nesx_mapper_t * Mapper;

} nesx_mmu_t;

void NESx_MMU_Init(nesx_t * ctx);
void NESx_MMU_Term(nesx_t * ctx);

uint8_t NESx_MMU_CPU_ReadByte(nesx_t * ctx, uint16_t address);
uint8_t NESx_MMU_PPU_ReadByte(nesx_t * ctx, uint16_t address);

void NESx_MMU_CPU_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data);
void NESx_MMU_PPU_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data);

void NESx_MMU_CPU_Tick(nesx_t * ctx);
void NESx_MMU_PPU_Tick(nesx_t * ctx);

#endif // NESX_MMU_H