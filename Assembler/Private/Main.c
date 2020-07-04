#include <stdio.h>
#include <stdint.h>

#include <cflags.h>

typedef struct label
{
    char name[128];
    uint16_t address;

    struct label * next;

} label_t;

typedef struct state
{
    uint8_t * buffer;
    size_t length;

    label_t * first_label;

} state_t;

label_t * add_label(state_t * state, const char * name, uint16_t address)
{
    label_t ** last_label = &state->first_label;
    while (*last_label) {
        last_label = &(*last_label)->next;
    }

    *last_label = (label_t *)malloc(sizeof(label_t));
    if (!*last_label) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    strncpy((*last_label)->name, name, sizeof((*last_label)->name));
    (*last_label)->address = address;

    return *last_label;
}

bool process(state_t * state, const char * filename)
{
    FILE * fp = fopen(filename, "rt");
    if (!fp) {
        fprintf(stderr, "file not found: %s\n", filename);
        return false;
    }

    while (!feof(fp)) {
    }


    fclose(fp);
    return true;
}

void output(state_t * state, const char * filename)
{

}

int main(int argc, char ** argv) 
{
    state_t state;

    cflags_t * flags = cflags_init();

    bool help = false;
    cflags_add_bool(flags, 'h', "help", &help, "display this help and exit");

    const char * outputFilename = "out.mos6502";
    cflags_add_string(flags, 'o', "output", &outputFilename, "output filename");

    cflags_parse(flags, argc, argv);

    if (flags->argc == 0) {
        cflags_print_usage(flags, 
            "[OPTION]... FILENAME...",
            "The NESx MOS6502 Assembler",
            "Additional information about this program can be found by contacting:\n"
                "  sdl.slane@gmail.com");

        cflags_free(flags);
        return 0;
    }

    bool success = true;
    for (int i = 0; success && i < flags->argc; ++i) {
        success = process(&state, flags->argv[i]);
    }

    cflags_free(flags);

    if (!success) {
        fprintf(stderr, "failed to process input files\n");
        return 1;
    }

    output(&state, outputFilename);

    return 0;
}