
#include <NESx/MOS6502/MOS6502.h>

#include "unit.h"

mos6502_t cpu;

void setup()
{
    mos6502_init(&cpu);
    cpu.P.raw = 0x00;
}

UNIT_TEST(StatusRegister_C)
{
    cpu.P.C = true;
    unit_assert_true(cpu.P.raw & (1 << 0));

    cpu.P.C = false;
    unit_assert_false(cpu.P.raw & (1 << 0));

    cpu.P.raw |= (1 << 0);
    unit_assert_true(cpu.P.C);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.C);
}

UNIT_TEST(StatusRegister_Z)
{
    cpu.P.Z = true;
    unit_assert_true(cpu.P.raw & (1 << 1));

    cpu.P.Z = false;
    unit_assert_false(cpu.P.raw & (1 << 1));

    cpu.P.raw |= (1 << 1);
    unit_assert_true(cpu.P.Z);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.Z);
}

UNIT_TEST(StatusRegister_I)
{
    cpu.P.I = true;
    unit_assert_true(cpu.P.raw & (1 << 2));

    cpu.P.I = false;
    unit_assert_false(cpu.P.raw & (1 << 2));

    cpu.P.raw |= (1 << 2);
    unit_assert_true(cpu.P.I);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.I);
}

UNIT_TEST(StatusRegister_D)
{
    cpu.P.D = true;
    unit_assert_true(cpu.P.raw & (1 << 3));

    cpu.P.D = false;
    unit_assert_false(cpu.P.raw & (1 << 3));

    cpu.P.raw |= (1 << 3);
    unit_assert_true(cpu.P.D);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.D);
}

UNIT_TEST(StatusRegister_B)
{
    cpu.P.B = true;
    unit_assert_true(cpu.P.raw & (1 << 4));

    cpu.P.B = false;
    unit_assert_false(cpu.P.raw & (1 << 4));

    cpu.P.raw |= (1 << 4);
    unit_assert_true(cpu.P.B);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.B);
}

UNIT_TEST(StatusRegister_V)
{
    cpu.P.V = true;
    unit_assert_true(cpu.P.raw & (1 << 6));

    cpu.P.V = false;
    unit_assert_false(cpu.P.raw & (1 << 6));

    cpu.P.raw |= (1 << 6);
    unit_assert_true(cpu.P.V);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.V);
}

UNIT_TEST(StatusRegister_N)
{
    cpu.P.N = true;
    unit_assert_true(cpu.P.raw & (1 << 7));

    cpu.P.N = false;
    unit_assert_false(cpu.P.raw & (1 << 7));

    cpu.P.raw |= (1 << 7);
    unit_assert_true(cpu.P.N);

    cpu.P.raw &= 0;
    unit_assert_false(cpu.P.N);
}

UNIT_TEST(StatusRegister_All)
{
    cpu.P.raw = 0xFF;
    unit_assert_true(cpu.P.C);
    unit_assert_true(cpu.P.Z);
    unit_assert_true(cpu.P.I);
    unit_assert_true(cpu.P.D);
    unit_assert_true(cpu.P.B);
    unit_assert_true(cpu.P.V);
    unit_assert_true(cpu.P.N);

    cpu.P.raw = 0x00;
    unit_assert_false(cpu.P.C);
    unit_assert_false(cpu.P.Z);
    unit_assert_false(cpu.P.I);
    unit_assert_false(cpu.P.D);
    unit_assert_false(cpu.P.B);
    unit_assert_false(cpu.P.V);
    unit_assert_false(cpu.P.N);
}

UNIT_TEST_SUITE(StatusRegister)
{
    UNIT_SUITE_SETUP(&setup);

    UNIT_RUN_TEST(StatusRegister_C);
    UNIT_RUN_TEST(StatusRegister_Z);
    UNIT_RUN_TEST(StatusRegister_I);
    UNIT_RUN_TEST(StatusRegister_D);
    UNIT_RUN_TEST(StatusRegister_B);
    UNIT_RUN_TEST(StatusRegister_V);
    UNIT_RUN_TEST(StatusRegister_N);
    UNIT_RUN_TEST(StatusRegister_All);
}

int main(int argc, char ** argv)
{
    UNIT_RUN_SUITE(StatusRegister);
    UNIT_REPORT();
    return UNIT_EXIT_CODE;
}