
#include <stdint.h>
#include <string.h>

const char * program_in = 
"DATA = $055        \n"
"ADDR = $6655       \n"
"                   \n"
"LDA #DATA          \n"
"LDA *DATA          \n"
"LDA *DATA,X        \n"
"LDA ADDR           \n"
"LDA ADDR,X         \n"
"LDA ADDR,Y         \n"
"LDA (ADDR,X)       \n"
"LDA (ADDR),Y       \n";

uint8_t program_out[] = {
    0xA9, 0x55, 0xA5, 0x55, 0xB5, 0x55, 0xAD, 0x55, 
    0x66, 0xBD, 0x55, 0x66, 0xB9, 0x55, 0x66, 0xA1,
    0x55, 0xB1, 0x55, 
};

int main(int argc, char ** argv)
{
    uint8_t * result;
    size_t size;
    MOS6502_Assemble(program_in, &result, &size);

    if (size != sizeof(program_out)) {
        printf("Size mismatch\n");
        return 1;
    }

    if (0 != memcmp(result, program_out, size)) {
        printf("Code mismatch\n");
        return 1;
    }

    return 0;
}