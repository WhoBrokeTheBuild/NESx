#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <NESx/MMU.h>
#include <NESx/NESx.h>

typedef struct log_entry
{
    unsigned PC;
    unsigned A;
    unsigned X;
    unsigned Y;
    unsigned P;
    unsigned S;
    unsigned Cycle;

} log_entry_t;

void parseLogEntry(char * line, log_entry_t * entry)
{
    sscanf(line, "%X", &entry->PC);
    sscanf(line + 48,
        "A:%X X:%X Y:%X P:%X SP:%X PPU: %*d, %*d CYC:%d",
        &entry->A,
        &entry->X,
        &entry->Y,
        &entry->P,
        &entry->S,
        &entry->Cycle);
}

bool compareLogEntry(log_entry_t * guess, log_entry_t * check)
{
    if (guess->PC != check->PC) {
        printf("PC is %04X, should be %04X\n", guess->PC, check->PC);
        return false;
    }

    if (guess->A != check->A) {
        printf("A is %02X, should be %02X\n", guess->A, check->A);
        return false;
    }

    if (guess->X != check->X) {
        printf("X is %02X, should be %02X\n", guess->X, check->X);
        return false;
    }

    if (guess->Y != check->Y) {
        printf("Y is %02X, should be %02X\n", guess->Y, check->Y);
        return false;
    }

    if (guess->P != check->P) {
        printf("P is %02X (%c%c-%c%c%c%c), should be %02X (%c%c-%c%c%c%c)\n",
            guess->P,
            ((guess->P & (1 << 7)) ? 'N' : 'n'),
            ((guess->P & (1 << 6)) ? 'V' : 'v'),
            ((guess->P & (1 << 3)) ? 'D' : 'd'),
            ((guess->P & (1 << 2)) ? 'I' : 'i'),
            ((guess->P & (1 << 1)) ? 'Z' : 'z'),
            ((guess->P & (1 << 0)) ? 'C' : 'c'),
            check->P,
            ((check->P & (1 << 7)) ? 'N' : 'n'),
            ((check->P & (1 << 6)) ? 'V' : 'v'),
            ((check->P & (1 << 3)) ? 'D' : 'd'),
            ((check->P & (1 << 2)) ? 'I' : 'i'),
            ((check->P & (1 << 1)) ? 'Z' : 'z'),
            ((check->P & (1 << 0)) ? 'C' : 'c'));
        return false;
    }

    if (guess->S != check->S) {
        printf("S is %02X, should be %02X\n", guess->S, check->S);
        return false;
    }

    if (guess->Cycle != check->Cycle) {
        printf("Cycle is %u, should be %u\n", guess->Cycle, check->Cycle);
        return false;
    }

    return true;
}

int main(int argc, char ** argv)
{
    FILE * fp = fopen("nestest.log", "rt");
    if (!fp) {
        fprintf(stderr, "failed to open nestest.log\n");
        return 1;
    }

    nesx_t nes;
    if (!NESx_Init(&nes)) {
        return 1;
    }

    if (!NESx_ROM_Load(&nes, "nestest.nes")) {
        return 1;
    }

    // https://www.qmtpro.com/~nes/misc/nestest.txt

    nes.CPU.PC = 0xC000;
    nes.CPU.AB = nes.CPU.PC;
    MOS6502_SetStatusRegister(&nes.CPU, 0x24);

    char line[256];
    log_entry_t check;

    log_entry_t guess;
    guess.Cycle = 7;

    int status = 0;

    const unsigned TOTAL_CYCLES = 14559; // Official Tests
    // const unsigned TOTAL_CYCLES = 26554; // All Tests

    while (fgets(line, sizeof(line), fp)) {

        parseLogEntry(line, &check);

        guess.PC = nes.CPU.PC;
        guess.A = nes.CPU.A;
        guess.X = nes.CPU.X;
        guess.Y = nes.CPU.Y;
        guess.P = MOS6502_GetStatusRegister(&nes.CPU);
        guess.S = nes.CPU.S;

        if (!compareLogEntry(&guess, &check)) {
            status = 1;
            break;
        }

        printf("Progress %d/%d %3.2f%%\n",
            guess.Cycle,
            TOTAL_CYCLES,
            ((float)guess.Cycle / (float)TOTAL_CYCLES) * 100.0);

        printf("%s", line);
        fflush(stdout);

        if (guess.Cycle >= TOTAL_CYCLES) {
            break;
        }

        while (true) {
            NESx_Tick(&nes);

            ++guess.Cycle;
            if (nes.CPU.SYNC) {
                break;
            }
        }
    }

    NESx_Term(&nes);

    fclose(fp);

    return status;
}
