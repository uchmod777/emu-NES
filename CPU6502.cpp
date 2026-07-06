#include "CPU6502.hpp"



void CPU6502::I_ADC(uint8_t operand)
{
    uint16_t result = (uint16_t) A + (uint16_t) operand + (uint16_t) getFlag(C);

    // Carry: result exceeded 8-bit range
    setFlag(C, result > 0xFF);

    // Zero: lower 8 bits are zero
    setFlag(Z, (result & 0xFF) == 0);

    // Negative: bit 7 set
    setFlag(N, result & 0x80);

    // Overflow: signed overflow occurred
    // Positive + Positive = Negative, or Negative + Negative = Positive
    setFlag(V, (~((uint16_t)A ^ (uint16_t)operand) & ((uint16_t)A ^ result)) & 0x80);
    
    A = result & 0xFF;
}

void CPU6502::I_AND(uint8_t operand)
{
    A = A & operand;

    setFlag(Z, A == 0);

    setFlag(N, A & 0x80);
}

void CPU6502::I_ASL_ACC()
{
    setFlag(C, A & 0x80);

    A <<= 1;

    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_ASL(uint16_t addr)
{
    uint8_t operand = read(addr);
    
    setFlag(C, operand & 0x80);

    operand <<= 1;
    write(addr, operand);

    setFlag(Z, operand == 0);
    setFlag(N, operand & 0x80);
}

void CPU6502::I_BCC(int8_t offset)
{
    if (!getFlag(C))
    {
        cycles++;  // branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BCS(int8_t offset)
{
    if (getFlag(C))
    {
        cycles++;  // branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BEQ(int8_t offset)
{
    if (getFlag(Z))
    {
        cycles++;  // Branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BIT(uint8_t operand)
{
    uint8_t mask = A & operand;
    
    setFlag(Z, mask == 0);

    setFlag(V, operand & 0x40);

    setFlag(N, operand & 0x80);
}

void CPU6502::I_BMI(int8_t offset)
{
    if (getFlag(N))
    {
        cycles++;  // Branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BNE(int8_t offset)
{
    if (!getFlag(Z))
    {
        cycles++;  // Branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BPL(int8_t offset)
{
    if (!getFlag(N))
    {
        cycles++;  // Branch taken penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BRK()
{
    push((PC >> 8) & 0xFF);
    push(PC & 0xFF);
    push(P | 0x30);  // 4th bit (B) and 5th bit (U)
    setFlag(I, true);
    PC = read(0xFFFE) | (read(0xFFFF) << 8);
}

void CPU6502::I_BVC(int8_t offset)
{
    if (!getFlag(V))
    {
        cycles++;  // Branch penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_BVS(int8_t offset)
{
    if (getFlag(V))
    {
        cycles++;  // Branch penalty

        uint16_t target = PC + offset;
        if ((PC & 0xFF00) != (target & 0xFF00))
        {
            cycles++;  // Page boundary penalty
        }

        PC = target;
    }
}

void CPU6502::I_CLC()
{
    setFlag(C, false);
}

void CPU6502::I_CLD()
{
    setFlag(D, false);
}

void CPU6502::I_CLI()
{
    setFlag(I, false);
}

void CPU6502::I_CLV()
{
    setFlag(V, false);
}

void CPU6502::I_CMP(uint8_t operand)
{
    uint16_t result = (uint16_t)A - (uint16_t)operand;

    setFlag(C, A >= operand);

    setFlag(Z, A == operand);

    setFlag(N, result & 0x80);
}

void CPU6502::I_CPX(uint8_t operand)
{
    uint16_t result = (uint16_t)X - (uint16_t)operand;

    setFlag(C, X >= operand);
    setFlag(Z, X == operand);
    setFlag(N, result & 0x80);
}

void CPU6502::I_CPY(uint8_t operand)
{
    uint16_t result = (uint16_t)Y - (uint16_t)operand;

    setFlag(C, Y >= operand);
    setFlag(Z, Y == operand);
    setFlag(N, result & 0x80);
}

void CPU6502::I_DEC(uint16_t addr)
{
    uint8_t result = read(addr) - 1;
    write(addr, result);

    setFlag(Z, result == 0);
    setFlag(N, result & 0x80);
}

void CPU6502::I_DEX()
{
    X--;

    setFlag(Z, X == 0);
    setFlag(N, X & 0x80);
}

void CPU6502::I_DEY()
{
    Y--;

    setFlag(Z, Y == 0);
    setFlag(N, Y & 0x80);
}

void CPU6502::I_EOR(uint8_t operand)
{
    A = A ^ operand;

    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_INC(uint16_t addr)
{
    uint8_t result = read(addr) + 1;
    write(addr, result);

    setFlag(Z, result == 0);
    setFlag(N, result & 0x80);
}