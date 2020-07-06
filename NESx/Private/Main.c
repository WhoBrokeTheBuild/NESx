
#include <NESx/MMU.h>
#include <NESx/Macros.h>
#include <NESx/NESx.h>
#include <cflags.h>

#include "Debug.h"

int main(int argc, char ** argv)
{
    int status = 0;

    cflags_t * flags = cflags_init();

    bool help = false;
    cflags_add_bool(flags, 'h', "help", &help, "display this help and exit");

    int scale = 1;
    cflags_add_int(flags, 's', "scale", &scale, "Window scale, default is 1");

    cflags_flag_t * verbose = cflags_add_bool(flags,
        'v',
        "verbose",
        NULL,
        "Enables verbose output, repeat up to 4 times for more verbosity");
    NESX_UNUSED(verbose);

    cflags_parse(flags, argc, argv);

    if (help || flags->argc == 1) {
        cflags_print_usage(flags,
            "[OPTION]... ROM_FILENAME",
            "A Toy Nintendo Entertainment System Emulator",
            "Additional information about this program can be found by "
            "contacting:\n"
            "  sdl.slane@gmail.com");

        cflags_free(flags);
        return 0;
    }

    nesx_t nes;
    if (!NESx_Init(&nes)) {
        status = 1;
        goto cleanup;
    }

    if (!NESx_LoadROM(&nes, flags->argv[1])) {
        status = 1;
        goto cleanup;
    }

    NESx_PrintROMHeader(&nes);

    if (!DebugInit(&nes, flags->argc, flags->argv)) {
        status = 1;
        goto cleanup;
    }

cleanup:

    NESx_Term(&nes);
    cflags_free(flags);
    return status;
}
