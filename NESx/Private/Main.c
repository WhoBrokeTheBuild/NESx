
#include <NESx/MMU.h>
#include <NESx/Macros.h>
#include <NESx/NESx.h>
#include <cflags.h>

int main(int argc, char ** argv)
{
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

    if (help || flags->argc == 0) {
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
        NESx_Term(&nes);
        return 1;
    }

    if (!NESx_LoadROM(&nes, flags->argv[0])) {
        NESx_Term(&nes);
        return 1;
    }

    NESx_PrintROMHeader(&nes);

    nes.CPU.PCL = NESx_ReadByte(&nes, 0xFFFE);
    nes.CPU.PCH = NESx_ReadByte(&nes, 0xFFFF);
    printf("%04X\n", nes.CPU.PC);

    printf("%02X %02X\n",
        NESx_ReadByte(&nes, nes.CPU.PC),
        NESx_ReadByte(&nes, nes.CPU.PC + 1));

    NESx_Term(&nes);
    return 0;
}
