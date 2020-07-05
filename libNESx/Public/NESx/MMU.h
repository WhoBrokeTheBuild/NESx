#ifndef NESX_MMU_H
#define NESX_MMU_H

#include <stdint.h>

struct nesx;
typedef struct nesx nesx_t;

uint8_t NESx_ReadByte(nesx_t * ctx, uint16_t address);

void NESx_WriteByte(nesx_t * ctx, uint16_t address, uint8_t data);

#endif // NESX_MMU_H