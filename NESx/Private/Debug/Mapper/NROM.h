#ifndef DEBUG_MAPPER_NROM_H
#define DEBUG_MAPPER_NROM_H

#include "Debug/DebugWindow.h"

// static void _Debug_NROM(debug_ctx_t * ctx)
// {
//     debug_memory_view_add_region(ctx->memCPU,
//         0x0000, 0x2000,
//         ctx->nes->MMU.InternalRAM,
//         sizeof(ctx->nes->MMU.InternalRAM));

//     debug_memory_view_add_region(ctx->memCPU,
//         0x2000, 0x2008,
//         ctx->nes->ROM.PRGROM,
//         0x08); // TODO: Change Data

//     debug_memory_view_add_region(ctx->memCPU,
//         0x4000, 0x4018,
//         ctx->nes->ROM.PRGROM,
//         0x18); // TODO: Change Data

//     debug_memory_view_add_region(ctx->memCPU,
//         0x4020, 0xFFFF,
//         ctx->nes->ROM.PRGROM,
//         ctx->nes->ROM.PRGROMSize); // TODO:
// }

#endif // DEBUG_MAPPER_NROM_H