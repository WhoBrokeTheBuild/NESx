#include <NESx/MOS6502/MOS6502.h>

#include <stdio.h>
#include <stdlib.h>

void MOS6502_Init(mos6502_t * cpu)
{
    cpu->AB = 0x0000;
    cpu->DB = 0x00;

    cpu->IR = 0x0000;

    cpu->PC = 0x0000;
    cpu->AD = 0x0000;

    cpu->A = 0xAA;
    cpu->X = 0x00;
    cpu->Y = 0x00;
    cpu->S = 0xFD;

    cpu->P.N = false;
    cpu->P.V = false;
    cpu->P.B = true;
    cpu->P.D = false;
    cpu->P.I = true;
    cpu->P.Z = true;
    cpu->P.C = false;

    cpu->RW = MOS6502_RW_READ;
    cpu->SYNC = true;
    cpu->IRQ = false;
    cpu->NMI = false;
    cpu->RDY = false;
    cpu->RES = true;

    cpu->BCDEnabled = true;
}

/*
Heavily influenced by
https://github.com/floooh/chips/blob/master/chips/m6502.h
*/

#define _FETCH()                                                               \
    cpu->AB = cpu->PC;                                                         \
    cpu->SYNC = true

#define _STALL() cpu->AB = cpu->PC

#define _SET_NZ(V)                                                             \
    cpu->P.N = ((V) >> 7);                                                     \
    cpu->P.Z = ((V) == 0)

#define _SHIFT_LEFT(V)                                                         \
    cpu->P.C = ((V)&0x80);                                                     \
    (V) <<= 1;                                                                 \
    _SET_NZ((V))

#define _PUSH(V)                                                               \
    cpu->ABH = 0x01;                                                           \
    cpu->ABL = cpu->S--;                                                       \
    cpu->DB = (V);                                                             \
    cpu->RW = MOS6502_RW_WRITE

#define _PULL()                                                                \
    cpu->ABH = 0x01;                                                           \
    cpu->ABL = cpu->S++;

#define _OR()                                                                  \
    cpu->A |= cpu->DB;                                                         \
    _SET_NZ(cpu->A)

#define _EOR()                                                                 \
    cpu->A ^= cpu->DB;                                                         \
    _SET_NZ(cpu->A)

// Skip the next cycle if a page boundary is not crossed

#define _PAGE_BOUND_CHECK_SKIP(V) cpu->IR += (~(cpu->ABL + (V)) >> 9) & 1

// immediate

#define _IMM() cpu->AB = cpu->PC++;

// zeropage

#define _ZPG_0() cpu->AB = cpu->PC++

#define _ZPG_1()                                                               \
    cpu->ADH = 0x00;                                                           \
    cpu->ADL = cpu->DB;                                                        \
    cpu->AB = cpu->AD

// zeropage,X

#define _ZPG_2_X()                                                             \
    cpu->ADL += cpu->X;                                                        \
    cpu->AB = cpu->AD

// zeropage,Y

#define _ZPG_2_Y()                                                             \
    cpu->ADL += cpu->Y;                                                        \
    cpu->AB = cpu->AD

// absolute

#define _ABS_0() cpu->AB = cpu->PC++

#define _ABS_1()                                                               \
    cpu->AB = cpu->PC++;                                                       \
    cpu->ADL = cpu->DB

#define _ABS_2()                                                               \
    cpu->ADH = cpu->DB;                                                        \
    cpu->AB = cpu->AD

// absolute,X

#define _ABS_2_X()                                                             \
    _ABS_2();                                                                  \
    cpu->ABL += cpu->X

#define _ABS_3_X() cpu->AB = cpu->AD + cpu->X

// absolute,Y

#define _ABS_2_Y()                                                             \
    _ABS_2();                                                                  \
    cpu->ABL += cpu->Y

#define _ABS_3_Y() cpu->AB = cpu->AD + cpu->Y

// indirect

#define _IND_0() cpu->AB = cpu->PC++

#define _IND_1()                                                               \
    cpu->ADH = 0x00;                                                           \
    cpu->ADL = cpu->DB;                                                        \
    cpu->AB = cpu->AD

// (zeropage,X)

#define _IND_2_X()                                                             \
    cpu->ADL += cpu->X;                                                        \
    cpu->AB = cpu->AD

#define _IND_3_X()                                                             \
    ++cpu->ABL;                                                                \
    cpu->ADL = cpu->DB

#define _IND_4_X()                                                             \
    cpu->ADH = cpu->DB;                                                        \
    cpu->AB = cpu->AD

// (zeropage),Y

#define _IND_2_Y()                                                             \
    ++cpu->ABL;                                                                \
    cpu->ADL = cpu->DB

#define _IND_3_Y()                                                             \
    cpu->ADH = cpu->DB;                                                        \
    cpu->AB = cpu->AD;                                                         \
    cpu->ABL += cpu->Y

#define _IND_4_Y() cpu->AB = cpu->AD + cpu->Y

// branch

#define _BRANCH_0() cpu->AB = cpu->PC++

#define _BRANCH_1(TEST)                                                        \
    cpu->AB = cpu->PC;                                                         \
    cpu->AD = cpu->PC + (int8_t)cpu->DB;                                       \
    if (TEST) {                                                                \
        _FETCH();                                                              \
    }

#define _BRANCH_2()                                                            \
    cpu->ABH = cpu->PCH;                                                       \
    cpu->ABL = cpu->ADL;                                                       \
    /* Check for page boundary */                                              \
    if (cpu->ADH == cpu->PCH) {                                                \
        cpu->PC = cpu->AD;                                                     \
        /* irq */                                                              \
        /* nmi */                                                              \
        _FETCH();                                                              \
    }

#define _BRANCH_3() cpu->PC = cpu->AD

void MOS6502_Tick(mos6502_t * cpu)
{
    if (cpu->RW && !cpu->RDY) {
        return;
    }

    if (cpu->SYNC) {
        cpu->IR = cpu->DB << 3;
        cpu->SYNC = false;

        ++cpu->PC;
    }

    cpu->RW = MOS6502_RW_READ;
    switch (cpu->IR++) {
    // BRK
    case (0x00 << 3) | 0: cpu->AB = cpu->PC; break;
    case (0x00 << 3) | 1:
        if (/* flags crap? */ false) {
            ++cpu->PC;
        }
        cpu->ADH = 0x01;
        cpu->ADL = cpu->S--;
        cpu->DB = cpu->PCH;
        if (/* more flags crap? */ false) {
            cpu->RW = MOS6502_RW_WRITE;
        }
        break;
    case (0x00 << 3) | 2:
        cpu->ADH = 0x01;
        cpu->ADL = cpu->S--;
        cpu->DB = cpu->PCL;
        if (/* more flags crap? */ false) {
            cpu->RW = MOS6502_RW_WRITE;
        }
        break;
    case (0x00 << 3) | 3:
        cpu->ADH = 0x01;
        cpu->ADL = cpu->P.raw; // TODO: Set unused flag
        cpu->DB = cpu->PCL;
        if (/* even more flags crap?? */ false) {
            cpu->AD = 0xFFFC;
        }
        else {
            cpu->RW = MOS6502_RW_WRITE;
            if (/* seriously ?*/ false) {
                cpu->AD = 0xFFFA;
            }
            else {
                cpu->AD = 0xFFFE;
            }
        }
        break;
    case (0x00 << 3) | 4:
        cpu->AB = cpu->AD++;
        cpu->P.I = true;
        cpu->P.B = true;
        // brk_flags
        break;
    case (0x00 << 3) | 5:
        cpu->AB = cpu->AD;
        cpu->ADL = cpu->DB;
        break;
    case (0x00 << 3) | 6:
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // ORA (zeropage,X)
    case (0x01 << 3) | 0: _IND_0(); break;
    case (0x01 << 3) | 1: _IND_1(); break;
    case (0x01 << 3) | 2: _IND_2_X(); break;
    case (0x01 << 3) | 3: _IND_3_X(); break;
    case (0x01 << 3) | 4: _IND_4_X(); break;
    case (0x01 << 3) | 5:
        _OR();
        _FETCH();
        break;

    // ORA zeropage
    case (0x05 << 3) | 0: _ZPG_0(); break;
    case (0x05 << 3) | 1: _ZPG_1(); break;
    case (0x05 << 3) | 2:
        _OR();
        _FETCH();
        break;

    // ASL zeropage
    case (0x06 << 3) | 0: _ZPG_0(); break;
    case (0x06 << 3) | 1: _ZPG_1(); break;
    case (0x06 << 3) | 2:
        cpu->ADH = 0x00;
        cpu->ADL = cpu->DB;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x06 << 3) | 3:
        _SHIFT_LEFT(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x06 << 3) | 4: _FETCH(); break;

    // PHP
    case (0x08 << 3) | 0: _STALL(); break;
    case (0x08 << 3) | 1:
        // TODO: Set unused flag
        _PUSH(cpu->P.raw);
        break;
    case (0x08 << 3) | 2: _FETCH(); break;

    // ORA immediate
    case (0x09 << 3) | 0: _IMM(); break;
    case (0x09 << 3) | 1:
        _OR();
        _FETCH();
        break;

    // ASL
    case (0x0A << 3) | 0: _STALL(); break;
    case (0x0A << 3) | 1:
        _SHIFT_LEFT(cpu->A);
        _FETCH();
        break;

    // ORA absolute
    case (0x0D << 3) | 0: _ABS_0(); break;
    case (0x0D << 3) | 1: _ABS_1(); break;
    case (0x0D << 3) | 2: _ABS_2(); break;
    case (0x0D << 3) | 3:
        _OR();
        _FETCH();
        break;

    // ASL absolute
    case (0x0E << 3) | 0: _ABS_0(); break;
    case (0x0E << 3) | 1: _ABS_1(); break;
    case (0x0E << 3) | 2: _ABS_2(); break;
    case (0x0E << 3) | 3:
        cpu->AD = cpu->DB;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x0E << 3) | 4:
        _SHIFT_LEFT(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x0E << 3) | 5: _FETCH(); break;

    // BPL
    case (0x10 << 3) | 0: _BRANCH_0(); break;
    case (0x10 << 3) | 1: _BRANCH_1(!cpu->P.Z); break;
    case (0x10 << 3) | 2: _BRANCH_2(); break;
    case (0x10 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // ORA (zeropage),Y
    case (0x11 << 3) | 0: _IND_0(); break;
    case (0x11 << 3) | 1: _IND_1(); break;
    case (0x11 << 3) | 2: _IND_2_Y(); break;
    case (0x11 << 3) | 3:
        _IND_3_Y();
        _PAGE_BOUND_CHECK_SKIP(cpu->Y);
        break;
    case (0x11 << 3) | 4: _IND_4_Y(); break;
    case (0x11 << 3) | 5:
        _OR();
        _FETCH();
        break;

    // ORA zeropage,X
    case (0x15 << 3) | 0: _ZPG_0(); break;
    case (0x15 << 3) | 1: _ZPG_1(); break;
    case (0x15 << 3) | 2: _ZPG_2_X(); break;
    case (0x15 << 3) | 4:
        _OR();
        _FETCH();
        break;

    // CLC
    case (0x18 << 3) | 0: _STALL(); break;
    case (0x18 << 3) | 1:
        cpu->P.C = false;
        _FETCH();
        break;

    // ORA absolute,Y
    case (0x19 << 3) | 0: _ABS_0(); break;
    case (0x19 << 3) | 1: _ABS_1(); break;
    case (0x19 << 3) | 2: _ABS_2_Y(); break;
    case (0x19 << 3) | 3:
        _ABS_3_Y();
        _PAGE_BOUND_CHECK_SKIP(cpu->Y);
        break;
    case (0x19 << 3) | 4:
        _OR();
        _FETCH();
        break;

    // ORA absolute,X
    case (0x1D << 3) | 0: _ABS_0(); break;
    case (0x1D << 3) | 1: _ABS_1(); break;
    case (0x1D << 3) | 2:
        _ABS_2_X();
        _PAGE_BOUND_CHECK_SKIP(cpu->X);
        break;
    case (0x1D << 3) | 3: _ABS_3_X(); break;
    case (0x1D << 3) | 4:
        _OR();
        _FETCH();
        break;

    // JSR
    case (0x20 << 3) | 0: cpu->AB = cpu->PC++; break;
    case (0x20 << 3) | 1:
        cpu->ABH = 0x01;
        cpu->ADL = cpu->DB;
        cpu->ABL = cpu->S;
        break;
    case (0x20 << 3) | 2: _PUSH(cpu->PCH); break;
    case (0x20 << 3) | 3: _PUSH(cpu->PCL); break;
    case (0x20 << 3) | 4: cpu->AB = cpu->PC; break;
    case (0x20 << 3) | 5:
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // BIT zeropage
    case (0x24 << 3) | 0: _ZPG_0(); break;
    case (0x24 << 3) | 1: _ZPG_1(); break;
    case (0x24 << 3) | 2:
        cpu->P.N = (cpu->DB >> 7);
        cpu->P.V = (cpu->DB >> 6);
        cpu->P.Z = ((cpu->A & cpu->DB) == 0);
        _FETCH();
        break;

    // PLP
    case (0x28 << 3) | 0: _PULL(); break;
    case (0x28 << 3) | 1:
        // Page Boundary
        cpu->ADL = cpu->S;
        break;
    case (0x28 << 3) | 2:
        cpu->P.raw = cpu->DB; // TODO: Unset unused flag
        cpu->P.B = true;
        _FETCH();
        break;

    // BIT absolute
    case (0x2C << 3) | 0: _ABS_0(); break;
    case (0x2C << 3) | 1: _ABS_1(); break;
    case (0x2C << 3) | 2: _ABS_2(); break;
    case (0x2C << 3) | 3:
        cpu->P.N = (cpu->DB >> 7);
        cpu->P.V = (cpu->DB >> 6);
        cpu->P.Z = ((cpu->A & cpu->DB) == 0);
        _FETCH();
        break;

    // BMI
    case (0x30 << 3) | 0: _BRANCH_0(); break;
    case (0x30 << 3) | 1: _BRANCH_1(!cpu->P.N); break;
    case (0x30 << 3) | 2: _BRANCH_2(); break;
    case (0x30 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // SEC
    case (0x38 << 3) | 0: _STALL(); break;
    case (0x38 << 3) | 1:
        cpu->P.C = true;
        _FETCH();
        break;

    // EOR (indirect,X)
    case (0x41 << 3) | 0: _IND_0(); break;
    case (0x41 << 3) | 1: _IND_1(); break;
    case (0x41 << 3) | 2: _IND_2_X(); break;
    case (0x41 << 3) | 3: _IND_3_X(); break;
    case (0x41 << 3) | 4: _IND_4_X(); break;
    case (0x41 << 3) | 5:
        _EOR();
        _FETCH();
        break;

    // EOR zeropage
    case (0x45 << 3) | 0: _ZPG_0(); break;
    case (0x45 << 3) | 1: _ZPG_1(); break;
    case (0x45 << 3) | 2:
        _EOR();
        _FETCH();
        break;

    // PHA
    case (0x48 << 3) | 0: _STALL(); break;
    case (0x48 << 3) | 1: _PUSH(cpu->A); break;
    case (0x48 << 3) | 2: _FETCH(); break;

    // EOR immediate
    case (0x49 << 3) | 0: _IMM(); break;
    case (0x49 << 3) | 1:
        _EOR();
        _FETCH();
        break;

    // JMP absolute
    case (0x4C << 3) | 0: cpu->AB = cpu->PC++; break;
    case (0x4C << 3) | 1:
        cpu->AB = cpu->PC++;
        cpu->ADL = cpu->DB;
        break;
    case (0x4C << 3) | 2:
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // EOR absolute
    case (0x4D << 3) | 0: _ABS_0(); break;
    case (0x4D << 3) | 1: _ABS_1(); break;
    case (0x4D << 3) | 2: _ABS_2(); break;
    case (0x4D << 3) | 3:
        _EOR();
        _FETCH();
        break;

    // BVC
    case (0x50 << 3) | 0: _BRANCH_0(); break;
    case (0x50 << 3) | 1: _BRANCH_1(cpu->P.V); break;
    case (0x50 << 3) | 2: _BRANCH_2(); break;
    case (0x50 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // EOR (indirect),Y
    case (0x51 << 3) | 0: _IND_0(); break;
    case (0x51 << 3) | 1: _IND_1(); break;
    case (0x51 << 3) | 2: _IND_2_Y(); break;
    case (0x51 << 3) | 3: _IND_3_Y(); break;
    case (0x51 << 3) | 4: _PAGE_BOUND_CHECK_SKIP(cpu->Y); break;
    case (0x51 << 3) | 5: _IND_4_Y(); break;
    case (0x51 << 3) | 6:
        _EOR();
        _FETCH();
        break;

    // EOR zeropage,X
    case (0x55 << 3) | 0: _ZPG_0(); break;
    case (0x55 << 3) | 1: _ZPG_1(); break;
    case (0x55 << 3) | 2: _ZPG_2_X(); break;
    case (0x55 << 3) | 3:
        _EOR();
        _FETCH();
        break;

    // CLI
    case (0x58 << 3) | 0: _STALL(); break;
    case (0x58 << 3) | 1:
        cpu->P.I = false;
        _FETCH();
        break;

    // EOR absolute,Y
    case (0x59 << 3) | 0: _ABS_0(); break;
    case (0x59 << 3) | 1: _ABS_1(); break;
    case (0x59 << 3) | 2:
        _ABS_2_Y();
        _PAGE_BOUND_CHECK_SKIP(cpu->Y);
        break;
    case (0x59 << 3) | 3: _ABS_3_Y(); break;
    case (0x59 << 3) | 4:
        _EOR();
        _FETCH();
        break;

    // EOR absolute,X
    case (0x5D << 3) | 0: _ABS_0(); break;
    case (0x5D << 3) | 1: _ABS_1(); break;
    case (0x5D << 3) | 2:
        _ABS_2_X();
        _PAGE_BOUND_CHECK_SKIP(cpu->X);
        break;
    case (0x5D << 3) | 3: _ABS_3_X(); break;
    case (0x5D << 3) | 4:
        _EOR();
        _FETCH();
        break;

    // RTS
    case (0x60 << 3) | 0: _STALL(); break;
    case (0x60 << 3) | 1: _PULL(); break;
    case (0x60 << 3) | 2: _PULL(); break;
    case (0x60 << 3) | 3:
        cpu->ABL = cpu->S;
        cpu->ADL = cpu->DB;
        break;
    case (0x60 << 3) | 4:
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        cpu->AB = cpu->PC++;
        break;
    case (0x60 << 3) | 5: _FETCH(); break;

    // PLA
    case (0x68 << 3) | 0: _STALL(); break;
    case (0x68 << 3) | 1: _PULL(); break;
    case (0x68 << 3) | 2: break;
    case (0x68 << 3) | 3:
        cpu->A = cpu->DB;
        _SET_NZ(cpu->A);
        _FETCH();
        break;

    // JMP (absolute)
    case (0x6C << 3) | 0: cpu->AB = cpu->PC++; break;
    case (0x6C << 3) | 1:
        cpu->AB = cpu->PC++;
        cpu->ADL = cpu->DB;
        break;
    case (0x6C << 3) | 2:
        cpu->ADH = cpu->DB;
        cpu->AB = cpu->AD;
        break;
    case (0x6C << 3) | 3:
        ++cpu->ADL;
        cpu->AB = cpu->AD;
        cpu->ADL = cpu->DB;
        break;
    case (0x6C << 3) | 4:
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // BVS
    case (0x70 << 3) | 0: _BRANCH_0(); break;
    case (0x70 << 3) | 1: _BRANCH_1(!cpu->P.V); break;
    case (0x70 << 3) | 2: _BRANCH_2(); break;
    case (0x70 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // SEI
    case (0x78 << 3) | 0: _STALL(); break;
    case (0x78 << 3) | 1:
        cpu->P.I = true;
        _FETCH();
        break;

    // STA (zeropage,X)
    case (0x81 << 3) | 0: _IND_0(); break;
    case (0x81 << 3) | 1: _IND_1(); break;
    case (0x81 << 3) | 2: _IND_2_X(); break;
    case (0x81 << 3) | 3: _IND_3_X(); break;
    case (0x81 << 3) | 4:
        _IND_4_X();
        cpu->DB = cpu->A;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x81 << 3) | 5: _FETCH(); break;

    // STX zeropage
    case (0x86 << 3) | 0: _ZPG_0(); break;
    case (0x86 << 3) | 1:
        _ZPG_1();
        cpu->DB = cpu->X;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x86 << 3) | 2: _FETCH(); break;

    // BCC
    case (0x90 << 3) | 0: _BRANCH_0(); break;
    case (0x90 << 3) | 1: _BRANCH_1(cpu->P.C); break;
    case (0x90 << 3) | 2: _BRANCH_2(); break;
    case (0x90 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // STA (zeropage),Y
    case (0x91 << 3) | 0: _IND_0(); break;
    case (0x91 << 3) | 1: _IND_1(); break;
    case (0x91 << 3) | 2: _IND_2_Y(); break;
    case (0x91 << 3) | 3: _IND_3_Y(); break;
    case (0x91 << 3) | 4:
        cpu->AB = cpu->AD + cpu->Y;
        cpu->DB = cpu->A;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x91 << 3) | 5: _FETCH(); break;

    // LDY immediate
    case (0xA0 << 3) | 0: _IMM(); break;
    case (0xA0 << 3) | 1:
        cpu->Y = cpu->DB;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // LDX immediate
    case (0xA2 << 3) | 0: _IMM(); break;
    case (0xA2 << 3) | 1:
        cpu->X = cpu->DB;
        _SET_NZ(cpu->X);
        _FETCH();
        break;

    // LDY zeropage
    case (0xA4 << 3) | 0: _ZPG_0(); break;
    case (0xA4 << 3) | 1: _ZPG_1(); break;
    case (0xA4 << 3) | 2:
        cpu->Y = cpu->DB;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // LDX zeropage
    case (0xA6 << 3) | 0: _ZPG_0(); break;
    case (0xA6 << 3) | 1: _ZPG_1(); break;
    case (0xA6 << 3) | 2:
        cpu->X = cpu->DB;
        _SET_NZ(cpu->X);
        _FETCH();
        break;

    // TAY
    case (0xA8 << 3) | 0: _STALL(); break;
    case (0xA8 << 3) | 1:
        cpu->Y = cpu->A;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // LDA immediate
    case (0xA9 << 3) | 0: _IMM(); break;
    case (0xA9 << 3) | 1:
        cpu->A = cpu->DB;
        _SET_NZ(cpu->A);
        _FETCH();
        break;

    // LDY absolute
    case (0xAC << 3) | 0: _ABS_0(); break;
    case (0xAC << 3) | 1: _ABS_1(); break;
    case (0xAC << 3) | 2: _ABS_2(); break;
    case (0xAC << 3) | 3:
        cpu->Y = cpu->DB;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // LDX absolute
    case (0xAE << 3) | 0: _ABS_0(); break;
    case (0xAE << 3) | 1: _ABS_1(); break;
    case (0xAE << 3) | 2: _ABS_2(); break;
    case (0xAE << 3) | 3:
        cpu->X = cpu->DB;
        _SET_NZ(cpu->X);
        _FETCH();
        break;

    // BCS
    case (0xB0 << 3) | 0: _BRANCH_0(); break;
    case (0xB0 << 3) | 1: _BRANCH_1(!cpu->P.C); break;
    case (0xB0 << 3) | 2: _BRANCH_2(); break;
    case (0xB0 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // LDY zeropage,X
    case (0xB4 << 3) | 0: _ZPG_0(); break;
    case (0xB4 << 3) | 1: _ZPG_1(); break;
    case (0xB4 << 3) | 2: _ZPG_2_X(); break;
    case (0xB4 << 3) | 3:
        cpu->Y = cpu->DB;
        _SET_NZ(cpu->Y);
        break;

    // STA zeropage
    case (0x85 << 3) | 0: _ZPG_0(); break;
    case (0x85 << 3) | 1:
        _ZPG_1();
        cpu->DB = cpu->A;
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0x85 << 3) | 2: _FETCH(); break;

    // LDX zeropage,Y
    case (0xB6 << 3) | 0: _ZPG_0(); break;
    case (0xB6 << 3) | 1: _ZPG_1(); break;
    case (0xB6 << 3) | 2: _ZPG_2_Y(); break;
    case (0xB6 << 3) | 3:
        cpu->X = cpu->DB;
        _SET_NZ(cpu->X);
        break;

    // CLV
    case (0xB8 << 3) | 0: _STALL(); break;
    case (0xB8 << 3) | 1:
        cpu->P.V = false;
        _FETCH();
        break;

    // LDY absolute,X
    case (0xBC << 3 | 0): _ABS_0(); break;
    case (0xBC << 3 | 1): _ABS_1(); break;
    case (0xBC << 3 | 2): _ABS_2_X(); break;
    case (0xBC << 3 | 3):
        cpu->Y = cpu->DB;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // LDX absolute,Y
    case (0xBE << 3 | 0): _ABS_0(); break;
    case (0xBE << 3 | 1): _ABS_1(); break;
    case (0xBE << 3 | 2): _ABS_2_Y(); break;
    case (0xBE << 3 | 3):
        cpu->X = cpu->DB;
        _SET_NZ(cpu->X);
        _FETCH();
        break;

    // BNE
    case (0xD0 << 3) | 0: _BRANCH_0(); break;
    case (0xD0 << 3) | 1: _BRANCH_1(cpu->P.Z); break;
    case (0xD0 << 3) | 2: _BRANCH_2(); break;
    case (0xD0 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // CLD
    case (0xD8 << 3) | 0: _STALL(); break;
    case (0xD8 << 3) | 1:
        cpu->P.D = false;
        _FETCH();
        break;

    // INC zeropage
    case (0xE6 << 3) | 0: _ZPG_0(); break;
    case (0xE6 << 3) | 1: _ZPG_1(); break;
    case (0xE6 << 3) | 2: cpu->RW = MOS6502_RW_WRITE; break;
    case (0xE6 << 3) | 3:
        ++cpu->DB;
        _SET_NZ(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0xE6 << 3) | 4: _FETCH(); break;

    // INY
    case (0xC8 << 3) | 0: _STALL(); break;
    case (0xC8 << 3) | 1:
        ++cpu->Y;
        _SET_NZ(cpu->Y);
        _FETCH();
        break;

    // INX
    case (0xE8 << 3) | 0: _STALL(); break;
    case (0xE8 << 3) | 1:
        ++cpu->X;
        _SET_NZ(cpu->X);
        _FETCH();
        break;

    // NOP
    case (0xEA << 3) | 0: _STALL(); break;
    case (0xEA << 3) | 1: _FETCH(); break;

    // INC absolute
    case (0xEE << 3) | 0: _ABS_0(); break;
    case (0xEE << 3) | 1: _ABS_1(); break;
    case (0xEE << 3) | 2: _ABS_2(); break;
    case (0xEE << 3) | 3: cpu->RW = MOS6502_RW_WRITE; break;
    case (0xEE << 3) | 4:
        ++cpu->DB;
        _SET_NZ(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0xEE << 3) | 5: _FETCH(); break;

    // BEQ
    case (0xF0 << 3) | 0: _BRANCH_0(); break;
    case (0xF0 << 3) | 1: _BRANCH_1(!cpu->P.Z); break;
    case (0xF0 << 3) | 2: _BRANCH_2(); break;
    case (0xF0 << 3) | 3:
        _BRANCH_3();
        _FETCH();
        break;

    // INC zeropage,X
    case (0xF6 << 3) | 0: _ZPG_0(); break;
    case (0xF6 << 3) | 1: _ZPG_1(); break;
    case (0xF6 << 3) | 2: _ZPG_2_X(); break;
    case (0xF6 << 3) | 3: cpu->RW = MOS6502_RW_WRITE; break;
    case (0xF6 << 3) | 4:
        ++cpu->DB;
        _SET_NZ(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0xF6 << 3) | 5: _FETCH(); break;

    // SED
    case (0xF8 << 3) | 0: _STALL(); break;
    case (0xF8 << 3) | 1:
        cpu->P.D = true;
        _FETCH();
        break;

    // INC absolute,X
    case (0xFE << 3) | 0: _ABS_0(); break;
    case (0xFE << 3) | 1: _ABS_1(); break;
    case (0xFE << 3) | 2: _ABS_2_X(); break;
    case (0xFE << 3) | 3: _ABS_3_X(); break;
    case (0xFE << 3) | 4: cpu->RW = MOS6502_RW_WRITE; break;
    case (0xFE << 3) | 5:
        ++cpu->DB;
        _SET_NZ(cpu->DB);
        cpu->RW = MOS6502_RW_WRITE;
        break;
    case (0xFE << 3) | 6: _FETCH(); break;

    default: assert(false); break;
    };
}