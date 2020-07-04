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

// DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
// DB7 & DB5 = LD
// DB0,1,2 = 1 = A
// DB0,1,2 = 2 = X
// DB1,1,2 = 0 = Y


// LDA immediate        0xA9    101 010 01
// LDA zeropage         0xA5    101 001 01
// LDA zeropage,X       0xB5    101 101 01
// LDA absolute         0xAD    101 011 01
// LDA absolute,X       0xBD    101 111 01
// LDA absolute,Y       0xB9    101 110 01
// LDA (indirect,X)     0xA1    101 000 01
// LDA (indirect),Y     0xB1    101 100 01

// LDX immediate        0xA2    101 000 10
// LDX zeropage         0xA6    101 001 10
// LDX zeropage,Y       0xB6    101 101 10
// LDX absolute         0xAE    101 011 10
// LDX absolute,Y       0xBE    101 111 10

// LDY immediate        0xA0    101 000 00
// LDY zeropage         0xA4    101 001 00
// LDY zeropage,X       0xB4    101 101 00
// LDY absolute         0xAC    101 011 00
// LDY absolute,X       0xBC    101 111 00

// TAY                  0xA8    101 010 00
// TAX                  0xAA    101 010 10

// TXS                  0x9A    100 110 10

bool process(state_t * state, const char * filename)
{
    FILE * fp = fopen(filename, "rt");
    if (!fp) {
        fprintf(stderr, "file not found: %s\n", filename);
        return false;
    }

    bool immediate = false;
    bool zeropage = false;
    bool absolute = false;
    bool indirect = false;
    bool xIndexed = false;
    bool yIndexed = false;

    #define A_ORA 0b000
    #define A_AND 0b001
    #define A_EOR 0b010
    #define A_ADC 0b011
    #define A_STA 0b100
    #define A_LDA 0b101
    #define A_CMP 0b110
    #define A_SBC 0b111

    #define B_IND_X 0b000
    #define B_ZPG   0b001
    #define B_IMM   0b010
    #define B_ABS   0b011
    #define B_IND_Y 0b100
    #define B_ZPG_X 0b101
    #define B_ABS_Y 0b110
    #define B_ABS_X 0b111

    typedef union opcode
    {
        struct {
            uint8_t a:3;
            uint8_t b:3;
            uint8_t c:2;
        };
        uint8_t raw;
    } opcode_t;

    opcode_t op;

    char inst[3] = { fgetc(fp), fgetc(fp), fgetc(fp) };



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