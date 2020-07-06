#include <NESx/MOS6502/MOS6502.h>

#include <stdio.h>
#include <stdlib.h>

/*
Heavily influenced by
https://github.com/floooh/chips/blob/master/chips/m6502.h
*/

#define _MASK_FC (1 << 0)
#define _MASK_FZ (1 << 1)
#define _MASK_FI (1 << 2)
#define _MASK_FD (1 << 3)
#define _MASK_FB (1 << 4)
#define _MASK_FX (1 << 5)
#define _MASK_FV (1 << 6)
#define _MASK_FN (1 << 7)

// clang-format off

#define _CYCLE(OP, CYC)                                                        \
    ((OP) << 3) | (CYC)

#define _FETCH()                                                               \
    cpu->AB = cpu->PC;                                                         \
    cpu->SYNC = true

#define _STALL()                                                               \
    cpu->AB = cpu->PC


// Flags

#define _SET_WRITE() \
    cpu->RW = 0
    
#define _SET_READ() \
    cpu->RW = 1

#define _SET_NZ_FLAGS(VALUE)                                                   \
    cpu->FN = ((VALUE) & 0x80);                                                \
    cpu->FZ = ((VALUE) == 0)

#define _GET_FLAGS()                                                           \
        (cpu->FC ? _MASK_FC : 0) |                                             \
        (cpu->FZ ? _MASK_FZ : 0) |                                             \
        (cpu->FI ? _MASK_FI : 0) |                                             \
        (cpu->FD ? _MASK_FD : 0) |                                             \
                   _MASK_FX      |                                             \
        (cpu->FV ? _MASK_FV : 0) |                                             \
        (cpu->FN ? _MASK_FN : 0)

#define _SET_FLAGS(VALUE)                                                      \
    cpu->FC = (((VALUE) & _MASK_FC) > 0);                                      \
    cpu->FZ = (((VALUE) & _MASK_FZ) > 0);                                      \
    cpu->FI = (((VALUE) & _MASK_FI) > 0);                                      \
    cpu->FD = (((VALUE) & _MASK_FD) > 0);                                      \
    cpu->FV = (((VALUE) & _MASK_FV) > 0);                                      \
    cpu->FN = (((VALUE) & _MASK_FN) > 0)

// Skip the next cycle if a page boundary is not crossed
#define _PAGE_BOUND_CHECK(V)                                                   \
    cpu->IR += (~(cpu->ADH - ((cpu->AD + (V)) >> 8))) & 1

// Addressing Modes

// immediate
#define _IMM()                                                                 \
    cpu->AB = cpu->PC++

// zeropage
#define _ZPG_0()                                                               \
    cpu->AB = cpu->PC++

#define _ZPG_1()                                                               \
    cpu->AB = cpu->DB

// zeropage,X
#define _ZPG_2_X()                                                             \
    cpu->ABL += cpu->X

// zeropage,Y
#define _ZPG_2_Y()                                                             \
    cpu->ABL += cpu->Y

// absolute
#define _ABS_0()                                                               \
    cpu->AB = cpu->PC++

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

#define _ABS_2_X_PGCHK()                                                       \
    _ABS_2_X();                                                                \
    _PAGE_BOUND_CHECK(cpu->X)

#define _ABS_3_X()                                                             \
    cpu->AB = cpu->AD + cpu->X

// absolute,Y
#define _ABS_2_Y()                                                             \
    _ABS_2();                                                                  \
    cpu->ABL += cpu->Y

#define _ABS_2_Y_PGCHK()                                                       \
    _ABS_2_Y();                                                                \
    _PAGE_BOUND_CHECK(cpu->Y)

#define _ABS_3_Y()                                                             \
    cpu->AB = cpu->AD + cpu->Y

// indirect
#define _IND_0()                                                               \
    cpu->AB = cpu->PC++

#define _IND_1()                                                               \
    cpu->AB = cpu->DB

// (indirect,X)
#define _IND_2_X()                                                             \
    cpu->ABL += cpu->X

#define _IND_3_X()                                                             \
    ++cpu->ABL;                                                                \
    cpu->ADL = cpu->DB

#define _IND_4_X()                                                             \
    cpu->ADH = cpu->DB;                                                        \
    cpu->AB = cpu->AD

// (indirect),Y
#define _IND_2_Y()                                                             \
    ++cpu->ABL;                                                                \
    cpu->ADL = cpu->DB

#define _IND_3_Y()                                                             \
    cpu->ADH = cpu->DB;                                                        \
    cpu->AB = cpu->AD;                                                         \
    cpu->ABL += cpu->Y

#define _IND_3_Y_PGCHK()                                                       \
    _IND_3_Y();                                                                \
    _PAGE_BOUND_CHECK(cpu->Y)

#define _IND_4_Y()                                                             \
    cpu->AB = cpu->AD + cpu->Y

// Branch

#define _BRANCH_0()                                                            \
    cpu->AB = cpu->PC++

#define _BRANCH_1(FAIL_TEST)                                                   \
    cpu->AB = cpu->PC;                                                         \
    cpu->AD = cpu->PC + (int8_t)cpu->DB;                                       \
    if (FAIL_TEST) {                                                           \
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

#define _BRANCH_3()                                                            \
    cpu->PC = cpu->AD

// Stack

#define _PUSH()                                                                \
    cpu->AB = (0x0100 | cpu->S--)

#define _PULL()                                                                \
    cpu->AB = (0x0100 | cpu->S++);

#define _PEEK()                                                                \
    cpu->AB = (0x0100 | cpu->S);

// clang-format on

// Instructions

#define _INC(VALUE)                                                            \
    ++(VALUE);                                                                 \
    _SET_NZ_FLAGS(VALUE)

#define _DEC(VALUE)                                                            \
    --(VALUE);                                                                 \
    _SET_NZ_FLAGS(VALUE)

#define _ASL(VALUE)                                                            \
    cpu->FC = ((VALUE)&0x80);                                                  \
    (VALUE) <<= 1;                                                             \
    _SET_NZ_FLAGS(VALUE)

#define _LSR(VALUE)                                                            \
    cpu->FC = ((VALUE)&0x01);                                                  \
    (VALUE) >>= 1;                                                             \
    _SET_NZ_FLAGS(VALUE)

#define _ROL(VALUE)                                                            \
    {                                                                          \
        uint16_t result = ((VALUE) << 1) | (cpu->FC ? 1 : 0);                  \
        cpu->FC = (result & 0x0100);                                           \
        (VALUE) = (result & 0xFF);                                             \
        _SET_NZ_FLAGS(VALUE);                                                  \
    }

#define _ROR(VALUE)                                                            \
    {                                                                          \
        uint16_t result = ((cpu->FC ? 0x100 : 0) | (VALUE)) >> 1;              \
        cpu->FC = ((VALUE)&0x01);                                              \
        (VALUE) = (result & 0xFF);                                             \
        _SET_NZ_FLAGS(VALUE);                                                  \
    }

#define _AND(VALUE)                                                            \
    cpu->A &= (VALUE);                                                         \
    _SET_NZ_FLAGS(cpu->A)

#define _ORA()                                                                 \
    cpu->A |= cpu->DB;                                                         \
    _SET_NZ_FLAGS(cpu->A)

#define _EOR()                                                                 \
    cpu->A ^= cpu->DB;                                                         \
    _SET_NZ_FLAGS(cpu->A)

#define _CMP(REG, VALUE)                                                       \
    {                                                                          \
        uint16_t result = (REG) - (VALUE);                                     \
        cpu->FC = !(result & 0xFF00);                                          \
        _SET_NZ_FLAGS(result & 0x00FF);                                        \
    }

#define _ADC(VALUE)                                                            \
    if (cpu->BCDEnabled && cpu->FD) {                                          \
        /* TODO: Support BCD */                                                \
    }                                                                          \
    else {                                                                     \
        uint16_t result = cpu->A + (VALUE) + (cpu->FC ? 1 : 0);                \
        cpu->FV = (~(cpu->A ^ (VALUE)) & (cpu->A ^ result) & 0x80);            \
        cpu->FC = (result & 0xFF00);                                           \
        cpu->A = (result & 0xFF);                                              \
        _SET_NZ_FLAGS(cpu->A);                                                 \
    }

#define _SBC(VALUE)                                                            \
    if (cpu->BCDEnabled && cpu->FD) {                                          \
        /* TODO: Support BCD */                                                \
    }                                                                          \
    else {                                                                     \
        uint16_t result = cpu->A - (VALUE) - (cpu->FC ? 0 : 1);                \
        cpu->FV = ((cpu->A ^ (VALUE)) & (cpu->A ^ result) & 0x80);             \
        cpu->FC = !(result & 0xFF00);                                          \
        cpu->A = (result & 0xFF);                                              \
        _SET_NZ_FLAGS(cpu->A);                                                 \
    }

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
    _SET_FLAGS(0x34);

    cpu->RW = 1;
    cpu->SYNC = 1;
    cpu->IRQ = 0;
    cpu->NMI = 0;
    cpu->RDY = 0;
    cpu->RES = 1;

    cpu->BCDEnabled = true;
}

void MOS6502_Tick(mos6502_t * cpu)
{
    if (cpu->RW && !cpu->RDY) {
        return;
    }

    if (cpu->SYNC) {
        cpu->IR = cpu->DB << 3;
        cpu->SYNC = 0;

        ++cpu->PC;
    }

    _SET_READ();
    switch (cpu->IR++) {
    // BRK
    case _CYCLE(0x00, 0): _STALL(); break;
    case _CYCLE(0x00, 1):
        if (/* flags crap? */ false) {
            ++cpu->PC;
        }
        _PUSH();
        cpu->DB = cpu->PCH;
        if (/* more flags crap? */ false) {
            _SET_WRITE();
        }
        break;
    case _CYCLE(0x00, 2):
        _PUSH();
        cpu->DB = cpu->PCL;
        if (/* more flags crap? */ false) {
            _SET_WRITE();
        }
        break;
    case _CYCLE(0x00, 3):
        cpu->ADH = 0x01;
        cpu->ADL = _GET_FLAGS() | _MASK_FB;
        cpu->DB = cpu->PCL;
        if (/* even more flags crap?? */ false) {
            cpu->AD = 0xFFFC;
        }
        else {
            _SET_WRITE();
            if (/* seriously ?*/ false) {
                cpu->AD = 0xFFFA;
            }
            else {
                cpu->AD = 0xFFFE;
            }
        }
        break;
    case _CYCLE(0x00, 4):
        cpu->AB = cpu->AD++;
        // cpu->FB = true;
        cpu->FI = true;
        // brk_flags
        break;
    case _CYCLE(0x00, 5):
        cpu->AB = cpu->AD;
        cpu->ADL = cpu->DB;
        break;
    case _CYCLE(0x00, 6):
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // ORA (indirect,X)
    case _CYCLE(0x01, 0): _IND_0(); break;
    case _CYCLE(0x01, 1): _IND_1(); break;
    case _CYCLE(0x01, 2): _IND_2_X(); break;
    case _CYCLE(0x01, 3): _IND_3_X(); break;
    case _CYCLE(0x01, 4): _IND_4_X(); break;
    case _CYCLE(0x01, 5):
        _ORA();
        _FETCH();
        break;

    // ORA zeropage
    case _CYCLE(0x05, 0): _ZPG_0(); break;
    case _CYCLE(0x05, 1): _ZPG_1(); break;
    case _CYCLE(0x05, 2):
        _ORA();
        _FETCH();
        break;

    // ASL zeropage
    case _CYCLE(0x06, 0): _ZPG_0(); break;
    case _CYCLE(0x06, 1): _ZPG_1(); break;
    case _CYCLE(0x06, 2): _SET_WRITE(); break;
    case _CYCLE(0x06, 3):
        _ASL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x06, 4): _FETCH(); break;

    // PHP
    case _CYCLE(0x08, 0): _STALL(); break;
    case _CYCLE(0x08, 1):
        _PUSH();
        cpu->DB = _GET_FLAGS() | _MASK_FB;
        _SET_WRITE();
        break;
    case _CYCLE(0x08, 2): _FETCH(); break;

    // ORA immediate
    case _CYCLE(0x09, 0): _IMM(); break;
    case _CYCLE(0x09, 1):
        _ORA();
        _FETCH();
        break;

    // ASL A
    case _CYCLE(0x0A, 0): _STALL(); break;
    case _CYCLE(0x0A, 1):
        _ASL(cpu->A);
        _FETCH();
        break;

    // ORA absolute
    case _CYCLE(0x0D, 0): _ABS_0(); break;
    case _CYCLE(0x0D, 1): _ABS_1(); break;
    case _CYCLE(0x0D, 2): _ABS_2(); break;
    case _CYCLE(0x0D, 3):
        _ORA();
        _FETCH();
        break;

    // ASL absolute
    case _CYCLE(0x0E, 0): _ABS_0(); break;
    case _CYCLE(0x0E, 1): _ABS_1(); break;
    case _CYCLE(0x0E, 2): _ABS_2(); break;
    case _CYCLE(0x0E, 3): _SET_WRITE(); break;
    case _CYCLE(0x0E, 4):
        _ASL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x0E, 5): _FETCH(); break;

    // BPL
    case _CYCLE(0x10, 0): _BRANCH_0(); break;
    case _CYCLE(0x10, 1): _BRANCH_1(cpu->FN); break;
    case _CYCLE(0x10, 2): _BRANCH_2(); break;
    case _CYCLE(0x10, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // ORA (indirect),Y
    case _CYCLE(0x11, 0): _IND_0(); break;
    case _CYCLE(0x11, 1): _IND_1(); break;
    case _CYCLE(0x11, 2): _IND_2_Y(); break;
    case _CYCLE(0x11, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0x11, 4): _IND_4_Y(); break;
    case _CYCLE(0x11, 5):
        _ORA();
        _FETCH();
        break;

    // ORA zeropage,X
    case _CYCLE(0x15, 0): _ZPG_0(); break;
    case _CYCLE(0x15, 1): _ZPG_1(); break;
    case _CYCLE(0x15, 2): _ZPG_2_X(); break;
    case _CYCLE(0x15, 3):
        _ORA();
        _FETCH();
        break;

    // ASL zeropage,X
    case _CYCLE(0x16, 0): _ZPG_0(); break;
    case _CYCLE(0x16, 1): _ZPG_1(); break;
    case _CYCLE(0x16, 2): _ZPG_2_X(); break;
    case _CYCLE(0x16, 3): _SET_WRITE(); break;
    case _CYCLE(0x16, 4):
        _ASL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x16, 5): _FETCH(); break;

    // CLC
    case _CYCLE(0x18, 0): _STALL(); break;
    case _CYCLE(0x18, 1):
        cpu->FC = 0;
        _FETCH();
        break;

    // ORA absolute,Y
    case _CYCLE(0x19, 0): _ABS_0(); break;
    case _CYCLE(0x19, 1): _ABS_1(); break;
    case _CYCLE(0x19, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0x19, 3): _ABS_3_Y(); break;
    case _CYCLE(0x19, 4):
        _ORA();
        _FETCH();
        break;

    // ORA absolute,X
    case _CYCLE(0x1D, 0): _ABS_0(); break;
    case _CYCLE(0x1D, 1): _ABS_1(); break;
    case _CYCLE(0x1D, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0x1D, 3): _ABS_3_X(); break;
    case _CYCLE(0x1D, 4):
        _ORA();
        _FETCH();
        break;

    // ASL absolute,X
    case _CYCLE(0x1E, 0): _ABS_0(); break;
    case _CYCLE(0x1E, 1): _ABS_1(); break;
    case _CYCLE(0x1E, 2): _ABS_2_X(); break;
    case _CYCLE(0x1E, 3): _ABS_3_X(); break;
    case _CYCLE(0x1E, 4): _SET_WRITE(); break;
    case _CYCLE(0x1E, 5):
        _ASL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x1E, 6): _FETCH(); break;

    // JSR
    case _CYCLE(0x20, 0): _ABS_0(); break;
    case _CYCLE(0x20, 1):
        _PEEK();
        cpu->ADL = cpu->DB;
        break;
    case _CYCLE(0x20, 2):
        _PUSH();
        _SET_WRITE();
        cpu->DB = cpu->PCH;
        break;
    case _CYCLE(0x20, 3):
        _PUSH();
        _SET_WRITE();
        cpu->DB = cpu->PCL;
        break;
    case _CYCLE(0x20, 4): _STALL(); break;
    case _CYCLE(0x20, 5):
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // AND (indirect,X)
    case _CYCLE(0x21, 0): _IND_0(); break;
    case _CYCLE(0x21, 1): _IND_1(); break;
    case _CYCLE(0x21, 2): _IND_2_X(); break;
    case _CYCLE(0x21, 3): _IND_3_X(); break;
    case _CYCLE(0x21, 4): _IND_4_X(); break;
    case _CYCLE(0x21, 5):
        _AND(cpu->DB);
        _FETCH();
        break;

    // BIT zeropage
    case _CYCLE(0x24, 0): _ZPG_0(); break;
    case _CYCLE(0x24, 1): _ZPG_1(); break;
    case _CYCLE(0x24, 2):
        cpu->FN = (cpu->DB & _MASK_FN);
        cpu->FV = (cpu->DB & _MASK_FV);
        cpu->FZ = ((cpu->A & cpu->DB) == 0);
        _FETCH();
        break;

    // AND zeropage
    case _CYCLE(0x25, 0): _ZPG_0(); break;
    case _CYCLE(0x25, 1): _ZPG_1(); break;
    case _CYCLE(0x25, 2):
        _AND(cpu->DB);
        _FETCH();
        break;

    // ROL zeropage
    case _CYCLE(0x26, 0): _ZPG_0(); break;
    case _CYCLE(0x26, 1): _ZPG_1(); break;
    case _CYCLE(0x26, 2): _SET_WRITE(); break;
    case _CYCLE(0x26, 3):
        _ROL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x26, 4): _FETCH(); break;

    // PLP
    case _CYCLE(0x28, 0): _STALL(); break;
    case _CYCLE(0x28, 1): _PULL(); break;
    case _CYCLE(0x28, 2): _PEEK(); break;
    case _CYCLE(0x28, 3):
        _SET_FLAGS(cpu->DB & ~_MASK_FB);
        _FETCH();
        break;

    // AND immediate
    case _CYCLE(0x29, 0): _IMM(); break;
    case _CYCLE(0x29, 1):
        _AND(cpu->DB);
        _FETCH();
        break;

    // ROL A
    case _CYCLE(0x2A, 0): _STALL(); break;
    case _CYCLE(0x2A, 1):
        _ROL(cpu->A);
        _FETCH();
        break;

    // BIT absolute
    case _CYCLE(0x2C, 0): _ABS_0(); break;
    case _CYCLE(0x2C, 1): _ABS_1(); break;
    case _CYCLE(0x2C, 2): _ABS_2(); break;
    case _CYCLE(0x2C, 3):
        cpu->FN = (cpu->DB & _MASK_FN);
        cpu->FV = (cpu->DB & _MASK_FV);
        cpu->FZ = ((cpu->A & cpu->DB) == 0);
        _FETCH();
        break;

    // AND absolute
    case _CYCLE(0x2D, 0): _ABS_0(); break;
    case _CYCLE(0x2D, 1): _ABS_1(); break;
    case _CYCLE(0x2D, 2): _ABS_2(); break;
    case _CYCLE(0x2D, 3):
        _AND(cpu->DB);
        _FETCH();
        break;

    // ROL absolute
    case _CYCLE(0x2E, 0): _ABS_0(); break;
    case _CYCLE(0x2E, 1): _ABS_1(); break;
    case _CYCLE(0x2E, 2): _ABS_2(); break;
    case _CYCLE(0x2E, 3): _SET_WRITE(); break;
    case _CYCLE(0x2E, 4):
        _ROL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x2E, 5): _FETCH(); break;

    // BMI
    case _CYCLE(0x30, 0): _BRANCH_0(); break;
    case _CYCLE(0x30, 1): _BRANCH_1(!cpu->FN); break;
    case _CYCLE(0x30, 2): _BRANCH_2(); break;
    case _CYCLE(0x30, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // AND (indirect),Y
    case _CYCLE(0x31, 0): _IND_0(); break;
    case _CYCLE(0x31, 1): _IND_1(); break;
    case _CYCLE(0x31, 2): _IND_2_Y(); break;
    case _CYCLE(0x31, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0x31, 4): _IND_4_Y(); break;
    case _CYCLE(0x31, 5):
        _AND(cpu->DB);
        _FETCH();
        break;

    // AND zeropage,X
    case _CYCLE(0x35, 0): _ZPG_0(); break;
    case _CYCLE(0x35, 1): _ZPG_1(); break;
    case _CYCLE(0x35, 2): _ZPG_2_X(); break;
    case _CYCLE(0x35, 3):
        _AND(cpu->DB);
        _FETCH();
        break;

    // ROL zeropage,X
    case _CYCLE(0x36, 0): _ZPG_0(); break;
    case _CYCLE(0x36, 1): _ZPG_1(); break;
    case _CYCLE(0x36, 2): _ZPG_2_X(); break;
    case _CYCLE(0x36, 3): _SET_WRITE(); break;
    case _CYCLE(0x36, 4):
        _ROL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x36, 5): _FETCH(); break;

    // SEC
    case _CYCLE(0x38, 0): _STALL(); break;
    case _CYCLE(0x38, 1):
        cpu->FC = true;
        _FETCH();
        break;

    // AND absolute,Y
    case _CYCLE(0x39, 0): _ABS_0(); break;
    case _CYCLE(0x39, 1): _ABS_1(); break;
    case _CYCLE(0x39, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0x39, 3): _ABS_3_Y(); break;
    case _CYCLE(0x39, 4):
        _AND(cpu->DB);
        _FETCH();
        break;

    // AND absolute,X
    case _CYCLE(0x3D, 0): _ABS_0(); break;
    case _CYCLE(0x3D, 1): _ABS_1(); break;
    case _CYCLE(0x3D, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0x3D, 3): _ABS_3_X(); break;
    case _CYCLE(0x3D, 4):
        _AND(cpu->DB);
        _FETCH();
        break;

    // ROL absolute,X
    case _CYCLE(0x3E, 0): _ABS_0(); break;
    case _CYCLE(0x3E, 1): _ABS_1(); break;
    case _CYCLE(0x3E, 2): _ABS_2_X(); break;
    case _CYCLE(0x3E, 3): _ABS_3_X(); break;
    case _CYCLE(0x3E, 4): _SET_WRITE(); break;
    case _CYCLE(0x3E, 5):
        _ROL(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x3E, 6): _FETCH(); break;

    // RTI
    case _CYCLE(0x40, 0): _STALL(); break;
    case _CYCLE(0x40, 1): _PULL(); break;
    case _CYCLE(0x40, 2): _PULL(); break;
    case _CYCLE(0x40, 3):
        _PULL();
        _SET_FLAGS(cpu->DB & ~_MASK_FB);
        break;
    case _CYCLE(0x40, 4):
        cpu->PCL = cpu->DB;
        _PEEK();
        break;
    case _CYCLE(0x40, 5):
        cpu->PCH = cpu->DB;
        _FETCH();
        break;

    // EOR (indirect,X)
    case _CYCLE(0x41, 0): _IND_0(); break;
    case _CYCLE(0x41, 1): _IND_1(); break;
    case _CYCLE(0x41, 2): _IND_2_X(); break;
    case _CYCLE(0x41, 3): _IND_3_X(); break;
    case _CYCLE(0x41, 4): _IND_4_X(); break;
    case _CYCLE(0x41, 5):
        _EOR();
        _FETCH();
        break;

    // EOR zeropage
    case _CYCLE(0x45, 0): _ZPG_0(); break;
    case _CYCLE(0x45, 1): _ZPG_1(); break;
    case _CYCLE(0x45, 2):
        _EOR();
        _FETCH();
        break;

    // LSR zeropage
    case _CYCLE(0x46, 0): _ZPG_0(); break;
    case _CYCLE(0x46, 1): _ZPG_1(); break;
    case _CYCLE(0x46, 2): break;
    case _CYCLE(0x46, 3):
        _LSR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x46, 4): _FETCH(); break;

    // PHA
    case _CYCLE(0x48, 0): _STALL(); break;
    case _CYCLE(0x48, 1):
        _PUSH();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x48, 2): _FETCH(); break;

    // EOR immediate
    case _CYCLE(0x49, 0): _IMM(); break;
    case _CYCLE(0x49, 1):
        _EOR();
        _FETCH();
        break;

    // LSR A
    case _CYCLE(0x4A, 0): _STALL(); break;
    case _CYCLE(0x4A, 1):
        _LSR(cpu->A);
        _FETCH();
        break;

    // JMP absolute
    case _CYCLE(0x4C, 0): _ABS_0(); break;
    case _CYCLE(0x4C, 1): _ABS_1(); break;
    case _CYCLE(0x4C, 2):
        _ABS_2();
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // EOR absolute
    case _CYCLE(0x4D, 0): _ABS_0(); break;
    case _CYCLE(0x4D, 1): _ABS_1(); break;
    case _CYCLE(0x4D, 2): _ABS_2(); break;
    case _CYCLE(0x4D, 3):
        _EOR();
        _FETCH();
        break;

    // LSR absolute
    case _CYCLE(0x4E, 0): _ABS_0(); break;
    case _CYCLE(0x4E, 1): _ABS_1(); break;
    case _CYCLE(0x4E, 2): _ABS_2(); break;
    case _CYCLE(0x4E, 3): _SET_WRITE(); break;
    case _CYCLE(0x4E, 4):
        _LSR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x4E, 5): _FETCH(); break;

    // BVC
    case _CYCLE(0x50, 0): _BRANCH_0(); break;
    case _CYCLE(0x50, 1): _BRANCH_1(cpu->FV); break;
    case _CYCLE(0x50, 2): _BRANCH_2(); break;
    case _CYCLE(0x50, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // EOR (indirect),Y
    case _CYCLE(0x51, 0): _IND_0(); break;
    case _CYCLE(0x51, 1): _IND_1(); break;
    case _CYCLE(0x51, 2): _IND_2_Y(); break;
    case _CYCLE(0x51, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0x51, 4): _IND_4_Y(); break;
    case _CYCLE(0x51, 5):
        _EOR();
        _FETCH();
        break;

    // EOR zeropage,X
    case _CYCLE(0x55, 0): _ZPG_0(); break;
    case _CYCLE(0x55, 1): _ZPG_1(); break;
    case _CYCLE(0x55, 2): _ZPG_2_X(); break;
    case _CYCLE(0x55, 3):
        _EOR();
        _FETCH();
        break;

    // LSR zeropage,X
    case _CYCLE(0x56, 0): _ZPG_0(); break;
    case _CYCLE(0x56, 1): _ZPG_1(); break;
    case _CYCLE(0x56, 2): _ZPG_2_X(); break;
    case _CYCLE(0x56, 3): _SET_WRITE(); break;
    case _CYCLE(0x56, 4):
        _LSR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x56, 5): _FETCH(); break;

    // CLI
    case _CYCLE(0x58, 0): _STALL(); break;
    case _CYCLE(0x58, 1):
        cpu->FI = false;
        _FETCH();
        break;

    // EOR absolute,Y
    case _CYCLE(0x59, 0): _ABS_0(); break;
    case _CYCLE(0x59, 1): _ABS_1(); break;
    case _CYCLE(0x59, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0x59, 3): _ABS_3_Y(); break;
    case _CYCLE(0x59, 4):
        _EOR();
        _FETCH();
        break;

    // EOR absolute,X
    case _CYCLE(0x5D, 0): _ABS_0(); break;
    case _CYCLE(0x5D, 1): _ABS_1(); break;
    case _CYCLE(0x5D, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0x5D, 3): _ABS_3_X(); break;
    case _CYCLE(0x5D, 4):
        _EOR();
        _FETCH();
        break;

    // LSR absolute,X
    case _CYCLE(0x5E, 0): _ABS_0(); break;
    case _CYCLE(0x5E, 1): _ABS_1(); break;
    case _CYCLE(0x5E, 2): _ABS_2_X(); break;
    case _CYCLE(0x5E, 3): _ABS_3_X(); break;
    case _CYCLE(0x5E, 4): _SET_WRITE(); break;
    case _CYCLE(0x5E, 5):
        _LSR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x5E, 6): _FETCH(); break;

    // RTS
    case _CYCLE(0x60, 0): _STALL(); break;
    case _CYCLE(0x60, 1): _PULL(); break;
    case _CYCLE(0x60, 2): _PULL(); break;
    case _CYCLE(0x60, 3):
        _PEEK();
        cpu->ADL = cpu->DB;
        break;
    case _CYCLE(0x60, 4):
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        cpu->AB = cpu->PC++;
        break;
    case _CYCLE(0x60, 5): _FETCH(); break;

    // ADC (indirect,X)
    case _CYCLE(0x61, 0): _IND_0(); break;
    case _CYCLE(0x61, 1): _IND_1(); break;
    case _CYCLE(0x61, 2): _IND_2_X(); break;
    case _CYCLE(0x61, 3): _IND_3_X(); break;
    case _CYCLE(0x61, 4): _IND_4_X(); break;
    case _CYCLE(0x61, 5):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ADC zeropage
    case _CYCLE(0x65, 0): _ZPG_0(); break;
    case _CYCLE(0x65, 1): _ZPG_1(); break;
    case _CYCLE(0x65, 2):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ROR zeropage
    case _CYCLE(0x66, 0): _ZPG_0(); break;
    case _CYCLE(0x66, 1): _ZPG_1(); break;
    case _CYCLE(0x66, 2): _SET_WRITE(); break;
    case _CYCLE(0x66, 3):
        _ROR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x66, 4): _FETCH(); break;

    // PLA
    case _CYCLE(0x68, 0): _STALL(); break;
    case _CYCLE(0x68, 1): _PULL(); break;
    case _CYCLE(0x68, 2): _PEEK(); break;
    case _CYCLE(0x68, 3):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // ADC immediate
    case _CYCLE(0x69, 0): _IMM(); break;
    case _CYCLE(0x69, 1):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ROR A
    case _CYCLE(0x6A, 0): _STALL(); break;
    case _CYCLE(0x6A, 1):
        _ROR(cpu->A);
        _FETCH();
        break;

    // JMP (absolute)
    case _CYCLE(0x6C, 0): _ABS_0(); break;
    case _CYCLE(0x6C, 1): _ABS_1(); break;
    case _CYCLE(0x6C, 2): _ABS_2(); break;
    case _CYCLE(0x6C, 3):
        cpu->ADL = cpu->DB;
        ++cpu->ABL;
        break;
    case _CYCLE(0x6C, 4):
        cpu->ADH = cpu->DB;
        cpu->PC = cpu->AD;
        _FETCH();
        break;

    // ADC absolute
    case _CYCLE(0x6D, 0): _ABS_0(); break;
    case _CYCLE(0x6D, 1): _ABS_1(); break;
    case _CYCLE(0x6D, 2): _ABS_2(); break;
    case _CYCLE(0x6D, 3):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ROR absolute
    case _CYCLE(0x6E, 0): _ABS_0(); break;
    case _CYCLE(0x6E, 1): _ABS_1(); break;
    case _CYCLE(0x6E, 2): _ABS_2(); break;
    case _CYCLE(0x6E, 3): _SET_WRITE(); break;
    case _CYCLE(0x6E, 4):
        _ROR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x6E, 5): _FETCH(); break;

    // BVS
    case _CYCLE(0x70, 0): _BRANCH_0(); break;
    case _CYCLE(0x70, 1): _BRANCH_1(!cpu->FV); break;
    case _CYCLE(0x70, 2): _BRANCH_2(); break;
    case _CYCLE(0x70, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // ADC (indirect),Y
    case _CYCLE(0x71, 0): _IND_0(); break;
    case _CYCLE(0x71, 1): _IND_1(); break;
    case _CYCLE(0x71, 2): _IND_2_Y(); break;
    case _CYCLE(0x71, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0x71, 4): _IND_4_Y(); break;
    case _CYCLE(0x71, 5):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ADC zeropage,X
    case _CYCLE(0x75, 0): _ZPG_0(); break;
    case _CYCLE(0x75, 1): _ZPG_1(); break;
    case _CYCLE(0x75, 2): _ZPG_2_X(); break;
    case _CYCLE(0x75, 3):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ROR zeropage,X
    case _CYCLE(0x76, 0): _ZPG_0(); break;
    case _CYCLE(0x76, 1): _ZPG_1(); break;
    case _CYCLE(0x76, 2): _ZPG_2_X(); break;
    case _CYCLE(0x76, 3): _SET_WRITE(); break;
    case _CYCLE(0x76, 4):
        _ROR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x76, 5): _FETCH(); break;

    // SEI
    case _CYCLE(0x78, 0): _STALL(); break;
    case _CYCLE(0x78, 1):
        cpu->FI = true;
        _FETCH();
        break;

    // ADC absolute,Y
    case _CYCLE(0x79, 0): _ABS_0(); break;
    case _CYCLE(0x79, 1): _ABS_1(); break;
    case _CYCLE(0x79, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0x79, 3): _ABS_3_Y(); break;
    case _CYCLE(0x79, 4):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ADC absolute,X
    case _CYCLE(0x7D, 0): _ABS_0(); break;
    case _CYCLE(0x7D, 1): _ABS_1(); break;
    case _CYCLE(0x7D, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0x7D, 3): _ABS_3_X(); break;
    case _CYCLE(0x7D, 4):
        _ADC(cpu->DB);
        _FETCH();
        break;

    // ROR absolute,X
    case _CYCLE(0x7E, 0): _ABS_0(); break;
    case _CYCLE(0x7E, 1): _ABS_1(); break;
    case _CYCLE(0x7E, 2): _ABS_2_X(); break;
    case _CYCLE(0x7E, 3): _ABS_3_X(); break;
    case _CYCLE(0x7E, 4): _SET_WRITE(); break;
    case _CYCLE(0x7E, 5):
        _ROR(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0x7E, 6): _FETCH(); break;

    // STA (indirect,X)
    case _CYCLE(0x81, 0): _IND_0(); break;
    case _CYCLE(0x81, 1): _IND_1(); break;
    case _CYCLE(0x81, 2): _IND_2_X(); break;
    case _CYCLE(0x81, 3): _IND_3_X(); break;
    case _CYCLE(0x81, 4):
        _IND_4_X();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x81, 5): _FETCH(); break;

    // STY zeropage
    case _CYCLE(0x84, 0): _ZPG_0(); break;
    case _CYCLE(0x84, 1):
        _ZPG_1();
        cpu->DB = cpu->Y;
        _SET_WRITE();
        break;
    case _CYCLE(0x84, 2): _FETCH(); break;

    // STA zeropage
    case _CYCLE(0x85, 0): _ZPG_0(); break;
    case _CYCLE(0x85, 1):
        _ZPG_1();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x85, 2): _FETCH(); break;

    // STX zeropage
    case _CYCLE(0x86, 0): _ZPG_0(); break;
    case _CYCLE(0x86, 1):
        _ZPG_1();
        cpu->DB = cpu->X;
        _SET_WRITE();
        break;
    case _CYCLE(0x86, 2): _FETCH(); break;

    // DEY
    case _CYCLE(0x88, 0): _STALL(); break;
    case _CYCLE(0x88, 1):
        _DEC(cpu->Y);
        _FETCH();
        break;

    // TXA
    case _CYCLE(0x8A, 0): _STALL(); break;
    case _CYCLE(0x8A, 1):
        cpu->A = cpu->X;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // STY absolute
    case _CYCLE(0x8C, 0): _ABS_0(); break;
    case _CYCLE(0x8C, 1): _ABS_1(); break;
    case _CYCLE(0x8C, 2):
        _ABS_2();
        cpu->DB = cpu->Y;
        _SET_WRITE();
        break;
    case _CYCLE(0x8C, 3): _FETCH(); break;

    // STA absolute
    case _CYCLE(0x8D, 0): _ABS_0(); break;
    case _CYCLE(0x8D, 1): _ABS_1(); break;
    case _CYCLE(0x8D, 2):
        _ABS_2();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x8D, 3): _FETCH(); break;

    // STX absolute
    case _CYCLE(0x8E, 0): _ABS_0(); break;
    case _CYCLE(0x8E, 1): _ABS_1(); break;
    case _CYCLE(0x8E, 2):
        _ABS_2();
        cpu->DB = cpu->X;
        _SET_WRITE();
        break;
    case _CYCLE(0x8E, 3): _FETCH(); break;

    // BCC
    case _CYCLE(0x90, 0): _BRANCH_0(); break;
    case _CYCLE(0x90, 1): _BRANCH_1(cpu->FC); break;
    case _CYCLE(0x90, 2): _BRANCH_2(); break;
    case _CYCLE(0x90, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // STA (indirect),Y
    case _CYCLE(0x91, 0): _IND_0(); break;
    case _CYCLE(0x91, 1): _IND_1(); break;
    case _CYCLE(0x91, 2): _IND_2_Y(); break;
    case _CYCLE(0x91, 3): _IND_3_Y(); break;
    case _CYCLE(0x91, 4):
        _IND_4_Y();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x91, 5): _FETCH(); break;

    // STY zeropage,X
    case _CYCLE(0x94, 0): _ZPG_0(); break;
    case _CYCLE(0x94, 1): _ZPG_1(); break;
    case _CYCLE(0x94, 2):
        _ZPG_2_X();
        cpu->DB = cpu->Y;
        _SET_WRITE();
        break;
    case _CYCLE(0x94, 3): _FETCH(); break;

    // STA zeropage,X
    case _CYCLE(0x95, 0): _ZPG_0(); break;
    case _CYCLE(0x95, 1): _ZPG_1(); break;
    case _CYCLE(0x95, 2):
        _ZPG_2_X();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x95, 3): _FETCH(); break;

    // STX zeropage,Y
    case _CYCLE(0x96, 0): _ZPG_0(); break;
    case _CYCLE(0x96, 1): _ZPG_1(); break;
    case _CYCLE(0x96, 2):
        _ZPG_2_Y();
        cpu->DB = cpu->X;
        _SET_WRITE();
        break;
    case _CYCLE(0x96, 3): _FETCH(); break;

    // TAX
    case _CYCLE(0x98, 0): _STALL(); break;
    case _CYCLE(0x98, 1):
        cpu->A = cpu->Y;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // STA absolute,Y
    case _CYCLE(0x99, 0): _ABS_0(); break;
    case _CYCLE(0x99, 1): _ABS_1(); break;
    case _CYCLE(0x99, 2): _ABS_2_Y(); break;
    case _CYCLE(0x99, 3):
        _ABS_3_Y();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x99, 4): _FETCH(); break;

    // TXS
    case _CYCLE(0x9A, 0): _STALL(); break;
    case _CYCLE(0x9A, 1):
        cpu->S = cpu->X;
        _FETCH();
        break;

    // STA absolute,X
    case _CYCLE(0x9D, 0): _ABS_0(); break;
    case _CYCLE(0x9D, 1): _ABS_1(); break;
    case _CYCLE(0x9D, 2): _ABS_2_X(); break;
    case _CYCLE(0x9D, 3):
        _ABS_3_X();
        cpu->DB = cpu->A;
        _SET_WRITE();
        break;
    case _CYCLE(0x9D, 4): _FETCH(); break;

    // LDY immediate
    case _CYCLE(0xA0, 0): _IMM(); break;
    case _CYCLE(0xA0, 1):
        cpu->Y = cpu->DB;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA (indirect,X)
    case _CYCLE(0xA1, 0): _IND_0(); break;
    case _CYCLE(0xA1, 1): _IND_1(); break;
    case _CYCLE(0xA1, 2): _IND_2_X(); break;
    case _CYCLE(0xA1, 3): _IND_3_X(); break;
    case _CYCLE(0xA1, 4): _IND_4_X(); break;
    case _CYCLE(0xA1, 5):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDX immediate
    case _CYCLE(0xA2, 0): _IMM(); break;
    case _CYCLE(0xA2, 1):
        cpu->X = cpu->DB;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // LDY zeropage
    case _CYCLE(0xA4, 0): _ZPG_0(); break;
    case _CYCLE(0xA4, 1): _ZPG_1(); break;
    case _CYCLE(0xA4, 2):
        cpu->Y = cpu->DB;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA zeropage
    case _CYCLE(0xA5, 0): _ZPG_0(); break;
    case _CYCLE(0xA5, 1): _ZPG_1(); break;
    case _CYCLE(0xA5, 2):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDX zeropage
    case _CYCLE(0xA6, 0): _ZPG_0(); break;
    case _CYCLE(0xA6, 1): _ZPG_1(); break;
    case _CYCLE(0xA6, 2):
        cpu->X = cpu->DB;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // TAY
    case _CYCLE(0xA8, 0): _STALL(); break;
    case _CYCLE(0xA8, 1):
        cpu->Y = cpu->A;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA immediate
    case _CYCLE(0xA9, 0): _IMM(); break;
    case _CYCLE(0xA9, 1):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // TAX
    case _CYCLE(0xAA, 0): _STALL(); break;
    case _CYCLE(0xAA, 1):
        cpu->X = cpu->A;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // LDY absolute
    case _CYCLE(0xAC, 0): _ABS_0(); break;
    case _CYCLE(0xAC, 1): _ABS_1(); break;
    case _CYCLE(0xAC, 2): _ABS_2(); break;
    case _CYCLE(0xAC, 3):
        cpu->Y = cpu->DB;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA absolute
    case _CYCLE(0xAD, 0): _ABS_0(); break;
    case _CYCLE(0xAD, 1): _ABS_1(); break;
    case _CYCLE(0xAD, 2): _ABS_2(); break;
    case _CYCLE(0xAD, 3):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDX absolute
    case _CYCLE(0xAE, 0): _ABS_0(); break;
    case _CYCLE(0xAE, 1): _ABS_1(); break;
    case _CYCLE(0xAE, 2): _ABS_2(); break;
    case _CYCLE(0xAE, 3):
        cpu->X = cpu->DB;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // BCS
    case _CYCLE(0xB0, 0): _BRANCH_0(); break;
    case _CYCLE(0xB0, 1): _BRANCH_1(!cpu->FC); break;
    case _CYCLE(0xB0, 2): _BRANCH_2(); break;
    case _CYCLE(0xB0, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // LDA (indirect),Y
    case _CYCLE(0xB1, 0): _IND_0(); break;
    case _CYCLE(0xB1, 1): _IND_1(); break;
    case _CYCLE(0xB1, 2): _IND_2_Y(); break;
    case _CYCLE(0xB1, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0xB1, 4): _IND_4_Y(); break;
    case _CYCLE(0xB1, 5):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDY zeropage,X
    case _CYCLE(0xB4, 0): _ZPG_0(); break;
    case _CYCLE(0xB4, 1): _ZPG_1(); break;
    case _CYCLE(0xB4, 2): _ZPG_2_X(); break;
    case _CYCLE(0xB4, 3):
        cpu->Y = cpu->DB;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA zeropage,X
    case _CYCLE(0xB5, 0): _ZPG_0(); break;
    case _CYCLE(0xB5, 1): _ZPG_1(); break;
    case _CYCLE(0xB5, 2): _ZPG_2_X(); break;
    case _CYCLE(0xB5, 3):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDX zeropage,Y
    case _CYCLE(0xB6, 0): _ZPG_0(); break;
    case _CYCLE(0xB6, 1): _ZPG_1(); break;
    case _CYCLE(0xB6, 2): _ZPG_2_Y(); break;
    case _CYCLE(0xB6, 3):
        cpu->X = cpu->DB;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // CLV
    case _CYCLE(0xB8, 0): _STALL(); break;
    case _CYCLE(0xB8, 1):
        cpu->FV = false;
        _FETCH();
        break;

    // LDA absolute,Y
    case _CYCLE(0xB9, 0): _ABS_0(); break;
    case _CYCLE(0xB9, 1): _ABS_1(); break;
    case _CYCLE(0xB9, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0xB9, 3): _ABS_3_Y(); break;
    case _CYCLE(0xB9, 4):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // TSX
    case _CYCLE(0xBA, 0): _STALL(); break;
    case _CYCLE(0xBA, 1):
        cpu->X = cpu->S;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // LDY absolute,X
    case _CYCLE(0xBC, 0): _ABS_0(); break;
    case _CYCLE(0xBC, 1): _ABS_1(); break;
    case _CYCLE(0xBC, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0xBC, 3): _ABS_3_X(); break;
    case _CYCLE(0xBC, 4):
        cpu->Y = cpu->DB;
        _SET_NZ_FLAGS(cpu->Y);
        _FETCH();
        break;

    // LDA absolute,X
    case _CYCLE(0xBD, 0): _ABS_0(); break;
    case _CYCLE(0xBD, 1): _ABS_1(); break;
    case _CYCLE(0xBD, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0xBD, 3): _ABS_3_X(); break;
    case _CYCLE(0xBD, 4):
        cpu->A = cpu->DB;
        _SET_NZ_FLAGS(cpu->A);
        _FETCH();
        break;

    // LDX absolute,Y
    case _CYCLE(0xBE, 0): _ABS_0(); break;
    case _CYCLE(0xBE, 1): _ABS_1(); break;
    case _CYCLE(0xBE, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0xBE, 3): _ABS_3_Y(); break;
    case _CYCLE(0xBE, 4):
        cpu->X = cpu->DB;
        _SET_NZ_FLAGS(cpu->X);
        _FETCH();
        break;

    // CPY immediate
    case _CYCLE(0xC0, 0): _IMM(); break;
    case _CYCLE(0xC0, 1):
        _CMP(cpu->Y, cpu->DB);
        _FETCH();
        break;

    // CMP (indirect,X)
    case _CYCLE(0xC1, 0): _IND_0(); break;
    case _CYCLE(0xC1, 1): _IND_1(); break;
    case _CYCLE(0xC1, 2): _IND_2_X(); break;
    case _CYCLE(0xC1, 3): _IND_3_X(); break;
    case _CYCLE(0xC1, 4): _IND_4_X(); break;
    case _CYCLE(0xC1, 5):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // CPY zeropage
    case _CYCLE(0xC4, 0): _ZPG_0(); break;
    case _CYCLE(0xC4, 1): _ZPG_1(); break;
    case _CYCLE(0xC4, 2):
        _CMP(cpu->Y, cpu->DB);
        _FETCH();
        break;

    // CMP zeropage
    case _CYCLE(0xC5, 0): _ZPG_0(); break;
    case _CYCLE(0xC5, 1): _ZPG_1(); break;
    case _CYCLE(0xC5, 2):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // DEC zeropage
    case _CYCLE(0xC6, 0): _ZPG_0(); break;
    case _CYCLE(0xC6, 1): _ZPG_1(); break;
    case _CYCLE(0xC6, 2): _SET_WRITE(); break;
    case _CYCLE(0xC6, 3):
        _DEC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xC6, 4): _FETCH(); break;

    // CMP immediate
    case _CYCLE(0xC9, 0): _IMM(); break;
    case _CYCLE(0xC9, 1):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // INY
    case _CYCLE(0xC8, 0): _STALL(); break;
    case _CYCLE(0xC8, 1):
        _INC(cpu->Y);
        _FETCH();
        break;

    // DEX
    case _CYCLE(0xCA, 0): _STALL(); break;
    case _CYCLE(0xCA, 1):
        _DEC(cpu->X);
        _FETCH();
        break;

    // CPY absolute
    case _CYCLE(0xCC, 0): _ABS_0(); break;
    case _CYCLE(0xCC, 1): _ABS_1(); break;
    case _CYCLE(0xCC, 2): _ABS_2(); break;
    case _CYCLE(0xCC, 3):
        _CMP(cpu->Y, cpu->DB);
        _FETCH();
        break;

    // CMP absolute
    case _CYCLE(0xCD, 0): _ABS_0(); break;
    case _CYCLE(0xCD, 1): _ABS_1(); break;
    case _CYCLE(0xCD, 2): _ABS_2(); break;
    case _CYCLE(0xCD, 3):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // DEC absolute
    case _CYCLE(0xCE, 0): _ABS_0(); break;
    case _CYCLE(0xCE, 1): _ABS_1(); break;
    case _CYCLE(0xCE, 2): _ABS_2(); break;
    case _CYCLE(0xCE, 3): _SET_WRITE(); break;
    case _CYCLE(0xCE, 4):
        _DEC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xCE, 5): _FETCH(); break;

    // BNE
    case _CYCLE(0xD0, 0): _BRANCH_0(); break;
    case _CYCLE(0xD0, 1): _BRANCH_1(cpu->FZ); break;
    case _CYCLE(0xD0, 2): _BRANCH_2(); break;
    case _CYCLE(0xD0, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // CMP (indirect),Y
    case _CYCLE(0xD1, 0): _IND_0(); break;
    case _CYCLE(0xD1, 1): _IND_1(); break;
    case _CYCLE(0xD1, 2): _IND_2_Y(); break;
    case _CYCLE(0xD1, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0xD1, 4): _IND_4_Y(); break;
    case _CYCLE(0xD1, 5):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // CMP zeropage,X
    case _CYCLE(0xD5, 0): _ZPG_0(); break;
    case _CYCLE(0xD5, 1): _ZPG_1(); break;
    case _CYCLE(0xD5, 2): _ZPG_2_X(); break;
    case _CYCLE(0xD5, 3):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // DEC zeropage,X
    case _CYCLE(0xD6, 0): _ZPG_0(); break;
    case _CYCLE(0xD6, 1): _ZPG_1(); break;
    case _CYCLE(0xD6, 2): _ZPG_2_X(); break;
    case _CYCLE(0xD6, 3): _SET_WRITE(); break;
    case _CYCLE(0xD6, 4):
        _DEC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xD6, 5): _FETCH(); break;

    // CLD
    case _CYCLE(0xD8, 0): _STALL(); break;
    case _CYCLE(0xD8, 1):
        cpu->FD = false;
        _FETCH();
        break;

    // CMP absolute,Y
    case _CYCLE(0xD9, 0): _ABS_0(); break;
    case _CYCLE(0xD9, 1): _ABS_1(); break;
    case _CYCLE(0xD9, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0xD9, 3): _ABS_3_Y(); break;
    case _CYCLE(0xD9, 4):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // CMP absolute,X
    case _CYCLE(0xDD, 0): _ABS_0(); break;
    case _CYCLE(0xDD, 1): _ABS_1(); break;
    case _CYCLE(0xDD, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0xDD, 3): _ABS_3_X(); break;
    case _CYCLE(0xDD, 4):
        _CMP(cpu->A, cpu->DB);
        _FETCH();
        break;

    // DEC absolute,X
    case _CYCLE(0xDE, 0): _ABS_0(); break;
    case _CYCLE(0xDE, 1): _ABS_1(); break;
    case _CYCLE(0xDE, 2): _ABS_2_X(); break;
    case _CYCLE(0xDE, 3): _ABS_3_X(); break;
    case _CYCLE(0xDE, 4): _SET_WRITE(); break;
    case _CYCLE(0xDE, 5):
        _DEC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xDE, 6): _FETCH(); break;

    // CPX immediate
    case _CYCLE(0xE0, 0): _IMM(); break;
    case _CYCLE(0xE0, 1):
        _CMP(cpu->X, cpu->DB);
        _FETCH();
        break;

    // SBC (indirect,X)
    case _CYCLE(0xE1, 0): _IND_0(); break;
    case _CYCLE(0xE1, 1): _IND_1(); break;
    case _CYCLE(0xE1, 2): _IND_2_X(); break;
    case _CYCLE(0xE1, 3): _IND_3_X(); break;
    case _CYCLE(0xE1, 4): _IND_4_X(); break;
    case _CYCLE(0xE1, 5):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // CPX zeropage
    case _CYCLE(0xE4, 0): _ZPG_0(); break;
    case _CYCLE(0xE4, 1): _ZPG_1(); break;
    case _CYCLE(0xE4, 2):
        _CMP(cpu->X, cpu->DB);
        _FETCH();
        break;

    // SBC zeropage
    case _CYCLE(0xE5, 0): _ZPG_0(); break;
    case _CYCLE(0xE5, 1): _ZPG_1(); break;
    case _CYCLE(0xE5, 2):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // INC zeropage
    case _CYCLE(0xE6, 0): _ZPG_0(); break;
    case _CYCLE(0xE6, 1): _ZPG_1(); break;
    case _CYCLE(0xE6, 2): _SET_WRITE(); break;
    case _CYCLE(0xE6, 3):
        _INC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xE6, 4): _FETCH(); break;

    // INX
    case _CYCLE(0xE8, 0): _STALL(); break;
    case _CYCLE(0xE8, 1):
        _INC(cpu->X);
        _FETCH();
        break;

    // SBC immediate
    case _CYCLE(0xE9, 0): _IMM(); break;
    case _CYCLE(0xE9, 1):
        _SBC(cpu->DB);
        _FETCH();
        break;
    // NOP
    case _CYCLE(0xEA, 0): _STALL(); break;
    case _CYCLE(0xEA, 1): _FETCH(); break;

    // INC absolute
    case _CYCLE(0xEE, 0): _ABS_0(); break;
    case _CYCLE(0xEE, 1): _ABS_1(); break;
    case _CYCLE(0xEE, 2): _ABS_2(); break;
    case _CYCLE(0xEE, 3): _SET_WRITE(); break;
    case _CYCLE(0xEE, 4):
        _INC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xEE, 5): _FETCH(); break;

    // CPX absolute
    case _CYCLE(0xEC, 0): _ABS_0(); break;
    case _CYCLE(0xEC, 1): _ABS_1(); break;
    case _CYCLE(0xEC, 2): _ABS_2(); break;
    case _CYCLE(0xEC, 3):
        _CMP(cpu->X, cpu->DB);
        _FETCH();
        break;

    // SBC absolute
    case _CYCLE(0xED, 0): _ABS_0(); break;
    case _CYCLE(0xED, 1): _ABS_1(); break;
    case _CYCLE(0xED, 2): _ABS_2(); break;
    case _CYCLE(0xED, 3):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // BEQ
    case _CYCLE(0xF0, 0): _BRANCH_0(); break;
    case _CYCLE(0xF0, 1): _BRANCH_1(!cpu->FZ); break;
    case _CYCLE(0xF0, 2): _BRANCH_2(); break;
    case _CYCLE(0xF0, 3):
        _BRANCH_3();
        _FETCH();
        break;

    // SBC (indirect),Y
    case _CYCLE(0xF1, 0): _IND_0(); break;
    case _CYCLE(0xF1, 1): _IND_1(); break;
    case _CYCLE(0xF1, 2): _IND_2_Y(); break;
    case _CYCLE(0xF1, 3): _IND_3_Y_PGCHK(); break;
    case _CYCLE(0xF1, 4): _IND_4_Y(); break;
    case _CYCLE(0xF1, 5):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // SBC zeropage,X
    case _CYCLE(0xF5, 0): _ZPG_0(); break;
    case _CYCLE(0xF5, 1): _ZPG_1(); break;
    case _CYCLE(0xF5, 2): _ZPG_2_X(); break;
    case _CYCLE(0xF5, 3):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // INC zeropage,X
    case _CYCLE(0xF6, 0): _ZPG_0(); break;
    case _CYCLE(0xF6, 1): _ZPG_1(); break;
    case _CYCLE(0xF6, 2): _ZPG_2_X(); break;
    case _CYCLE(0xF6, 3): _SET_WRITE(); break;
    case _CYCLE(0xF6, 4):
        _INC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xF6, 5): _FETCH(); break;

    // SED
    case _CYCLE(0xF8, 0): _STALL(); break;
    case _CYCLE(0xF8, 1):
        cpu->FD = true;
        _FETCH();
        break;

    // SBC absolute,Y
    case _CYCLE(0xF9, 0): _ABS_0(); break;
    case _CYCLE(0xF9, 1): _ABS_1(); break;
    case _CYCLE(0xF9, 2): _ABS_2_Y_PGCHK(); break;
    case _CYCLE(0xF9, 3): _ABS_3_Y(); break;
    case _CYCLE(0xF9, 4):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // SBC absolute,X
    case _CYCLE(0xFD, 0): _ABS_0(); break;
    case _CYCLE(0xFD, 1): _ABS_1(); break;
    case _CYCLE(0xFD, 2): _ABS_2_X_PGCHK(); break;
    case _CYCLE(0xFD, 3): _ABS_3_X(); break;
    case _CYCLE(0xFD, 4):
        _SBC(cpu->DB);
        _FETCH();
        break;

    // INC absolute,X
    case _CYCLE(0xFE, 0): _ABS_0(); break;
    case _CYCLE(0xFE, 1): _ABS_1(); break;
    case _CYCLE(0xFE, 2): _ABS_2_X(); break;
    case _CYCLE(0xFE, 3): _ABS_3_X(); break;
    case _CYCLE(0xFE, 4): _SET_WRITE(); break;
    case _CYCLE(0xFE, 5):
        _INC(cpu->DB);
        _SET_WRITE();
        break;
    case _CYCLE(0xFE, 6): _FETCH(); break;

    default:
        fprintf(stderr,
            "unsupported opcode/cycle %02X:%d\n",
            (cpu->IR >> 3),
            (cpu->IR & 0b111));
        assert(false);
        break;
    };
}

void MOS6502_SetStatusRegister(mos6502_t * cpu, uint8_t p)
{
    _SET_FLAGS(p);
}

uint8_t MOS6502_GetStatusRegister(mos6502_t * cpu)
{
    return _GET_FLAGS();
}