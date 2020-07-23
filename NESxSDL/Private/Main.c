#include <NESx/NESx.h>
#include <cflags.h>
#include <glad/gl.h>
#include <SDL.h>

int main(int argc, char ** argv)
{
    int status = 0;
    SDL_Window * window = NULL;
    SDL_GLContext context = NULL;

    cflags_t * flags = cflags_init();

    bool help = false;
    cflags_add_bool(flags, 'h', "help", &help, "display this help and exit");

    int scale = 1;
    cflags_add_int(flags, 's', "scale", &scale, "set the initial window scale, default is 1");

    cflags_flag_t * verbose = cflags_add_bool(flags, 'v', "verbose", NULL,
        "enables verbose output, repeat up to 4 times for more verbosity");
    
    cflags_parse(flags, argc, argv);

    if (help || flags->argc == 1) {
        cflags_print_usage(flags,
            "[OPTION]... ROM_FILENAME",
            "A Toy Nintendo Entertainment System Emulator (SDL Version)",
            "Additional information about this program can be found by contacting:\n"
            "  sdl.slane@gmail.com"
        );

        cflags_free(flags);
        return 0;
    }

    nesx_t nes;
    nes.Verbosity = verbose->count;
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("NESx",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        NESX_WIDTH * scale, NESX_HEIGHT * scale,
        SDL_WINDOW_OPENGL);

    if (!window) {
        fprintf(stderr, "failed to create SDL window: %s\n", SDL_GetError());
        status = 1;
        goto cleanup;
    }

    context = SDL_GL_CreateContext(window);
    
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "failed to initialize OpenGL context\n");
        status = 1;
        goto cleanup;
    }

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
    SDL_GL_SetSwapInterval(1);

    SDL_Event evt;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                running = false;
            }
        }

        NESx_Frame(&nes);
        SDL_GL_SwapWindow(window);
    }

cleanup:

    NESx_Term(&nes);

    cflags_free(flags);

    return status;
}