#include <stdio.h>
#include <string.h>

#include <MOS6502/MOS6502.h>

int main(int argc, char ** argv)
{
    uint8_t memory[1 << 16] = { 0x00 };

    // clang-format off

    uint8_t program[] = {
        0xA2, 0x55, // LDX $55
        0xA0, 0x66, // LDY $66
        0x38,       // SEC
        0xE8,       // INX
        0xC8,       // INY
        0x18,       // CLC

        // NOPs
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
        0xEA, 0xEA, 0xEA, 0xEA, 0xEA,
    };

    // clang-format on

    memcpy(memory, program, sizeof(program));

    mos6502_t cpu;
    mos6502_init(&cpu);

    for (int i = 0; i < 16; ++i) {
        if (cpu.RW == MOS6502_RW_READ) {
            cpu.DB = memory[cpu.AB];
            cpu.RDY = true;
        }
        else {
            memory[cpu.AB] = cpu.DB;
            cpu.RDY = true;
        }

        printf("%03d T%d RW=%d SYNC=%d RDY=%d IR=%02X PC=%04X S=%02X A=%02X "
               "X=%02X Y=%02X P=%c%c%c%c%c%c%c AD=%04X AB=%04X DB=%02X\n",
            i,
            (cpu.IR & 0x7),
            cpu.RW,
            cpu.SYNC,
            cpu.RDY,
            (cpu.IR >> 3),
            cpu.PC,
            cpu.S,
            cpu.A,
            cpu.X,
            cpu.Y,
            (cpu.P.N ? 'N' : 'n'),
            (cpu.P.V ? 'V' : 'v'),
            (cpu.P.B ? 'B' : 'b'),
            (cpu.P.D ? 'D' : 'd'),
            (cpu.P.I ? 'I' : 'i'),
            (cpu.P.Z ? 'Z' : 'z'),
            (cpu.P.C ? 'C' : 'c'),
            cpu.AD,
            cpu.AB,
            cpu.DB);

        mos6502_tick(&cpu);
    }

    return 0;
}
