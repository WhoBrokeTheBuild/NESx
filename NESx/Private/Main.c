
#include <NESx/MMU.h>
#include <NESx/Macros.h>
#include <NESx/NESx.h>
#include <cflags.h>

#include <SDL.h>
#include <glad/gl.h>

#if defined(NESX_DEBUGGER)
#    include "Debug.h"
#endif

int main(int argc, char ** argv)
{
    int status = 0;
    SDL_Window * window = NULL;
    SDL_GLContext glCtx = NULL;

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

    if (!NESx_ROM_Load(&nes, flags->argv[1])) {
        status = 1;
        goto cleanup;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "failed to init SDL: %s\n", SDL_GetError());
        status = 1;
        goto cleanup;
    }

    // clang-format off

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("NESx", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        256, 240,
        SDL_WINDOW_OPENGL);

    // clang-format on

    if (!window) {
        fprintf(stderr, "failed to create SDL window: %s\n", SDL_GetError());
        status = 1;
        goto cleanup;
    }

    glCtx = SDL_GL_CreateContext(window);

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "failed to initialize OpenGL context\n");
        status = 1;
        goto cleanup;
    }

    NESx_ROM_PrintHeader(&nes);

    nes.CPU.PC = 0xC000;
    nes.CPU.AB = nes.CPU.PC;
    MOS6502_SetStatusRegister(&nes.CPU, 0x24);

#if defined(NESX_DEBUGGER)

    if (!DebugRun(&nes, window, flags->argc, flags->argv)) {
        status = 1;
        goto cleanup;
    }

#else

    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        NESx_Frame(&nes);

        SDL_GL_SwapWindow(window);
    }

#endif

cleanup:

    SDL_GL_DeleteContext(glCtx);
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();

    NESx_Term(&nes);

    cflags_free(flags);

    return status;
}
