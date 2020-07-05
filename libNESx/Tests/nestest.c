#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <NESx/NESx.h>
#include <NESx/MMU.h>

typedef struct log_entry
{
    unsigned PC;
    unsigned A;
    unsigned X;
    unsigned Y;
    mos6502_flags_t P;
    unsigned S;
    unsigned Cycle;

} log_entry_t;

void parseLogEntry(char * line, log_entry_t * entry)
{
    sscanf(line, "%X", &entry->PC);
    sscanf(line + 48, "A:%X X:%X Y:%X P:%hhX SP:%X PPU: %*d, %*d CYC:%d",
        &entry->A,
        &entry->X,
        &entry->Y,
        &entry->P.raw,
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
        printf("A is %04X, should be %04X\n", guess->A, check->A);
        return false;
    }

    if (guess->X != check->X) {
        printf("X is %04X, should be %04X\n", guess->X, check->X);
        return false;
    }

    if (guess->Y != check->Y) {
        printf("Y is %04X, should be %04X\n", guess->Y, check->Y);
        return false;
    }

    if (guess->P.raw != check->P.raw) {
        printf("P is %04X (%c%c-%c%c%c%c%c), should be %04X (%c%c-%c%c%c%c%c)\n", 
            guess->P.raw,
            (guess->P.N ? 'N' : 'n'),
            (guess->P.V ? 'V' : 'v'),
            (guess->P.B ? 'B' : 'b'),
            (guess->P.D ? 'D' : 'd'),
            (guess->P.I ? 'I' : 'i'),
            (guess->P.Z ? 'Z' : 'z'),
            (guess->P.C ? 'C' : 'c'),
            check->P.raw,
            (check->P.N ? 'N' : 'n'),
            (check->P.V ? 'V' : 'v'),
            (check->P.B ? 'B' : 'b'),
            (check->P.D ? 'D' : 'd'),
            (check->P.I ? 'I' : 'i'),
            (check->P.Z ? 'Z' : 'z'),
            (check->P.C ? 'C' : 'c')
        );
        return false;
    }

    if (guess->S != check->S) {
        printf("S is %04X, should be %04X\n", guess->S, check->S);
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

    if (!NESx_LoadROM(&nes, "nestest.nes")) {
        return 1;
    }

    // https://www.qmtpro.com/~nes/misc/nestest.txt
    
    nes.CPU.PC = 0xC000;
    nes.CPU.AB = nes.CPU.PC;
    nes.CPU.P.raw = 0x24; // nestest quirk

    char line[256];
    log_entry_t check;

    log_entry_t guess;
    guess.Cycle = 7;

    bool success = true;

    while (fgets(line, sizeof(line), fp)) {
        
        parseLogEntry(line, &check);

        guess.PC = nes.CPU.PC;
        guess.A = nes.CPU.A;
        guess.X = nes.CPU.X;
        guess.Y = nes.CPU.Y;
        guess.P.raw = nes.CPU.P.raw;
        guess.S = nes.CPU.S;

        if (!compareLogEntry(&guess, &check)) {
            success = false;
            break;
        }

        printf("%s", line);

        while (true) {
            if (nes.CPU.RW) {
                nes.CPU.DB = NESx_ReadByte(&nes, nes.CPU.AB);
                nes.CPU.RDY = true;
            }
            else {
                NESx_WriteByte(&nes, nes.CPU.AB, nes.CPU.DB);
                nes.CPU.RDY = true;
            }

            MOS6502_Tick(&nes.CPU);

            ++guess.Cycle;
            if (nes.CPU.SYNC) {
                break;
            }
        }
    }

    NESx_Term(&nes);

    fclose(fp);

    return 0;
}
