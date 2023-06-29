//file test_suite1.c
#define CLOVE_SUITE_NAME MySuite01
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "clove-unit.h"
#include "Chipotto.h"

CLOVE_SUITE_SETUP_ONCE()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) 
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    }
}

CLOVE_TEST(JP_test)
{
    chipotto::Emulator emulator;
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode1(0x1300));
    CLOVE_INT_EQ((int)0x300 - 2, (int)emulator.GetPCCounter());
}

CLOVE_TEST(CALL_test)
{
	chipotto::Emulator emulator;
    for (uint8_t i = 0; i <= (int)0xF; ++i)
    {
        CLOVE_INT_EQ((int)chipotto::OpcodeStatus::NotIncrementPC, (int)emulator.Opcode2(0x2300));
        CLOVE_INT_EQ((int)i, (int)emulator.GetSPCounter());
        CLOVE_INT_EQ((int)0x0300, (int)emulator.GetPCCounter());
	}
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::StackOverflow, (int)emulator.Opcode2(0x2300));
    CLOVE_INT_EQ(0xF, (int)emulator.GetSPCounter());
}

CLOVE_TEST(SE_test)
{
    chipotto::Emulator emulator;
    uint16_t myPC = (int)emulator.GetPCCounter();
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode3(0x3300));
    CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());

    myPC = (int)emulator.GetPCCounter();
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode3(0x3401));
    CLOVE_INT_EQ((int)myPC, (int)emulator.GetPCCounter());
}

CLOVE_TEST(SNE_test)
{
    chipotto::Emulator emulator;
    uint16_t myPC = (int)emulator.GetPCCounter();
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode4(0x4300));
    CLOVE_INT_EQ((int)myPC, (int)emulator.GetPCCounter());

    myPC = (int)emulator.GetPCCounter();
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode4(0x4401));
    CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());
}

CLOVE_TEST(SE_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	uint16_t myPC = (int)emulator.GetPCCounter();
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode5(0x5300));
	CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());

    emulator.Opcode6(0x6745);
    emulator.Opcode6(0x6865);
    myPC = (int)emulator.GetPCCounter();
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode5(0x5780));
    CLOVE_INT_EQ((int)myPC, (int)emulator.GetPCCounter());
}

CLOVE_TEST(LD_test)
{
	chipotto::Emulator emulator;
    uint16_t myPC = (int)emulator.GetPCCounter();
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode6(0x6345));
    emulator.Opcode3(0x3345);
    CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());

    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode6(0x6455));
    myPC = (int)emulator.GetPCCounter();
    emulator.Opcode4(0x4445);
    CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());

}

CLOVE_TEST(ADD_test)
{
    chipotto::Emulator emulator;
    uint16_t myPC = (int)emulator.GetPCCounter();

    emulator.Opcode6(0x6101);
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode7(0x7144));
    emulator.Opcode3(0x3145);
    CLOVE_INT_EQ(myPC + 2, (int)emulator.GetPCCounter());


}

CLOVE_TEST(LD_Vx_Vy_test)
{
    chipotto::Emulator emulator;
    emulator.Opcode6(0x6012); // Y Register[4] = 56
    emulator.Opcode6(0x6304); // X Register[3] = 4
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8300));
    CLOVE_INT_EQ(0x0012, (int)emulator.GetRegisters(3));
}

CLOVE_TEST(OR_Vx_Vy_test)
{
    chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x6308); // X Register[3] = 8
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8341));
    CLOVE_INT_EQ(0x0056 | 0x0008, (int)emulator.GetRegisters(3));
}

CLOVE_TEST(AND_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8342));
    CLOVE_INT_EQ(0x0056 & 0x000C, (int)emulator.GetRegisters(3));
}

CLOVE_TEST(XOR_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8343));
    CLOVE_INT_EQ(0x0056 ^ 0x000C, (int)emulator.GetRegisters(3));
}

CLOVE_TEST(ADD_Vx_Vy_test)
{
    chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8344));
	CLOVE_INT_EQ(0x0056 + 0x000C, (int)emulator.GetRegisters(3));
    CLOVE_INT_EQ(0x0, (int)emulator.GetRegisters(0xF));

    emulator.Opcode6(0x64FF); // Y Register[4] = 0xFF
    emulator.Opcode6(0x630C); // X Register[3] = 0xC
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8344));
    CLOVE_INT_EQ(0x0B, (int)emulator.GetRegisters(3));
    CLOVE_INT_EQ(0x1, (int)emulator.GetRegisters(0xF));
}

CLOVE_TEST(SUB_Vx_Vy_test)
{
    chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8345));
	CLOVE_INT_EQ(0x100 - 0x4A, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x0, (int)emulator.GetRegisters(0xF));

	emulator.Opcode6(0x640C); // Y Register[4] = 0xC
	emulator.Opcode6(0x6356); // X Register[3] = 0x56
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8345));
    CLOVE_INT_EQ(0x4A, (int)emulator.GetRegisters(3));
    CLOVE_INT_EQ(0x1, (int)emulator.GetRegisters(0xF));
}

CLOVE_TEST(SHR_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8346));
	CLOVE_INT_EQ(0x6, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x0, (int)emulator.GetRegisters(0xF));

	emulator.Opcode6(0x630D); // X Register[3] = 0xD
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8346));
	CLOVE_INT_EQ(0x6, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x1, (int)emulator.GetRegisters(0xF));
}

CLOVE_TEST(SUBN_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8347));
	CLOVE_INT_EQ(0x4A, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x1, (int)emulator.GetRegisters(0xF));

	emulator.Opcode6(0x640C); // Y Register[4] = 0xC
	emulator.Opcode6(0x6356); // X Register[3] = 0x56
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x8347));
	CLOVE_INT_EQ(0x100 - 0x4A, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x0, (int)emulator.GetRegisters(0xF));
}

CLOVE_TEST(SHL_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	emulator.Opcode6(0x6456); // Y Register[4] = 56
	emulator.Opcode6(0x630C); // X Register[3] = 0xC
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x834E));
	CLOVE_INT_EQ(0x18, (int)emulator.GetRegisters(3));
	CLOVE_INT_EQ(0x0, (int)emulator.GetRegisters(0xF));

    emulator.Opcode6(0x63FD); // X Register[3] = 0xFD
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode8(0x834E));
    CLOVE_INT_EQ(0xFA, (int)emulator.GetRegisters(3));
    CLOVE_INT_EQ(0x1, (int)emulator.GetRegisters(0xF));
}

CLOVE_TEST(SNE_Vx_Vy_test)
{
	chipotto::Emulator emulator;
	uint16_t myPC = (int)emulator.GetPCCounter();
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode9(0x9340));
	CLOVE_INT_EQ((int)myPC, (int)emulator.GetPCCounter());

	emulator.Opcode6(0x6745);
	emulator.Opcode6(0x6865);
	myPC = (int)emulator.GetPCCounter();
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.Opcode9(0x9780));
	CLOVE_INT_EQ((int)myPC + 2, (int)emulator.GetPCCounter());
}

CLOVE_TEST(LD_I_test)
{
    chipotto::Emulator emulator;
	CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.OpcodeA(0xA001));
    CLOVE_INT_EQ(0x0001, emulator.GetI());
}

CLOVE_TEST(JP_V0_test)
{
    chipotto::Emulator emulator;
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.OpcodeB(0xB300));
    CLOVE_INT_EQ((int)0x0300 - 2, (int)emulator.GetPCCounter());
    emulator.Opcode6(0x6002);
    CLOVE_INT_EQ((int)chipotto::OpcodeStatus::IncrementPC, (int)emulator.OpcodeB(0xB002));
    CLOVE_INT_EQ((int)0x0004 - 2, (int)emulator.GetPCCounter());
}

//CLOVE_TEST(RND_test) 
//{
//
//}
//
//CLOVE_TEST(DRW_test)
//{
//
//}

CLOVE_TEST(FirstTest) {
    int a = 1;
    int b = 1;
    CLOVE_INT_EQ(a, b);
}

CLOVE_TEST(SecondTest) {
    CLOVE_IS_TRUE(1);
}