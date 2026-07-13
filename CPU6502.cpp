#include "CPU6502.hpp"

void CPU6502::LoadMemory(uint16_t addr, uint8_t data)
{
    write(addr, data);
}

void CPU6502::LogCPU(std::ofstream& logFile, uint64_t totalCycles)
{
    uint8_t opcode = memory[PC];
    uint8_t len    = instructionSizes[opcode];

    // Build raw bytes string dynamically based on length
    char bytes[9] = {0};
    if (len == 1)
    {
        snprintf(bytes, sizeof(bytes), "%02X      ", opcode);
    } else if (len == 2)
    {
        snprintf(bytes, sizeof(bytes), "%02X %02X   ", opcode, memory[PC+1]);
    } else if (len == 3)
    {
        snprintf(bytes, sizeof(bytes), "%02X %02X %02X", opcode, memory[PC+1], memory[PC+2]);
    }

    logFile << std::uppercase << std::hex << std::setfill('0')
            << std::setw(4) << PC << "  "
            << bytes << "  "
            << "A:"  << std::setw(2) << (int)A
            << " X:" << std::setw(2) << (int)X
            << " Y:" << std::setw(2) << (int)Y
            << " P:" << std::setw(2) << (int)P
            << " SP:" << std::setw(2) << (int)SP
            << " CYC:" << std::dec << totalCycles
            << "\n";

}

void CPU6502::Reset()
{
    A = 0;
    X = 0;
    Y = 0;
    SP = 0xFD;
    P = 0x24;
    PC = read(0xFFFC) | (read(0xFFFD) << 8);
    cycles = 8;
}

void CPU6502::NMI()
{
    push((PC >> 8) & 0xFF);
    push(PC & 0xFF);
    push(P | 0x20);  // U bit set, B bit clear for hardware interrupt
    setFlag(I, true);
    PC = read(0xFFFA) | (read(0xFFFB) << 8);
    cycles = 8;
}

void CPU6502::IRQ()
{
    if (!getFlag(I))
    {
        push((PC >> 8) & 0xFF);
        push(PC & 0xFF);
        push(P | 0x20);  // U bit set, B bit clear for hardware interrupt
        setFlag(I, true);
        PC = read(0xFFFE) | (read(0xFFFF) << 8);
        cycles = 7;
    }
}

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

void CPU6502::I_INX()
{
    X = X + 1;

    setFlag(Z, X == 0);
    setFlag(N, X & 0x80);
}

void CPU6502::I_INY()
{
    Y = Y + 1;

    setFlag(Z, Y == 0);
    setFlag(N, Y & 0x80);
}

void CPU6502::I_JMP(uint16_t addr)
{
    PC = addr;
}

void CPU6502::I_JSR(uint16_t addr)
{
    uint16_t ret = PC - 1;
    push((ret >> 8) & 0xFF);
    push(ret & 0xFF);
    
    PC = addr;
}

void CPU6502::I_LDA(uint8_t operand)
{
    A = operand;

    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_LDX(uint8_t operand)
{
    X = operand;

    setFlag(Z, X == 0);
    setFlag(N, X & 0x80);
}

void CPU6502::I_LDY(uint8_t operand)
{
    Y = operand;

    setFlag(Z, Y == 0);
    setFlag(N, Y & 0x80);
}

void CPU6502::I_LSR(uint16_t addr)
{
    uint8_t operand = read(addr);

    setFlag(C, operand & 0x01);
    
    operand >>= 1;
    write(addr, operand);

    setFlag(Z, operand == 0);
    setFlag(N, false);
}

void CPU6502::I_LSR_ACC()
{
    setFlag(C, A & 0x01);
    A >>= 1;

    setFlag(Z, A == 0);
    setFlag(N, false);
}

void CPU6502::I_NOP()
{

}

void CPU6502::I_ORA(uint8_t operand)
{
    A = A | operand;

    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_PHA()
{
    push(A);
}

void CPU6502::I_PHP()
{
    push(P | 0x30);
}

void CPU6502::I_PLA()
{
    A = pop();

    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_PLP()
{
    P = pop() & 0xCF;
}

void CPU6502::I_ROL_ACC()
{
    uint8_t oldCarry = getFlag(C);
    setFlag(C, A & 0x80);
    A <<= 1;
    A = A | oldCarry;
    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_ROL(uint16_t addr)
{
    uint8_t operand = read(addr);
    uint8_t oldCarry = getFlag(C);
    setFlag(C, operand & 0x80);
    operand <<= 1;
    operand |= oldCarry;
    write(addr, operand);
    setFlag(Z, operand == 0);
    setFlag(N, operand & 0x80);
}

void CPU6502::I_ROR_ACC()
{
    uint8_t oldCarry = getFlag(C);
    setFlag(C, A & 0x01);
    A >>= 1;
    A |= (oldCarry << 7);  // shift into bit position 7
    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_ROR(uint16_t addr)
{
    uint8_t operand = read(addr);
    uint8_t oldCarry = getFlag(C);
    setFlag(C, operand & 0x01);  // 0-bit moves into carry
    operand >>= 1;
    operand |= (oldCarry << 7);
    write(addr, operand);
    setFlag(Z, operand == 0);
    setFlag(N, operand & 0x80);
}

void CPU6502::I_RTI()
{
    P = pop() & 0xCF;
    uint8_t lowByte = pop();
    uint8_t highByte = pop();
    PC = (highByte << 8) | lowByte;
}

void CPU6502::I_RTS()
{
    uint8_t lowByte = pop();
    uint8_t highByte = pop();
    PC = ((highByte << 8) | lowByte) + 1;
}

void CPU6502::I_SBC(uint8_t operand)
{
    uint16_t result = (uint16_t)A - (uint16_t)operand - (1 - getFlag(C));
    setFlag(C, result < 0x100);
    setFlag(Z, (result & 0xFF) == 0);
    setFlag(V, (result ^ A) & (result ^ ~operand) & 0x80);
    setFlag(N, result & 0x80);
    A = result & 0xFF;
}

void CPU6502::I_SEC()
{
    setFlag(C, true);
}

void CPU6502::I_SED()
{
    setFlag(D, true);
}

void CPU6502::I_SEI()
{
    setFlag(I, true);
}

void CPU6502::I_STA(uint16_t addr)
{
    write(addr, A);
}

void CPU6502::I_STX(uint16_t addr)
{
    write(addr, X);
}

void CPU6502::I_STY(uint16_t addr)
{
    write(addr, Y);
}

void CPU6502::I_TAX()
{
    X = A;
    setFlag(Z, X == 0);
    setFlag(N, X & 0x80);
}

void CPU6502::I_TAY()
{
    Y = A;
    setFlag(Z, Y == 0);
    setFlag(N, Y & 0x80);
}

void CPU6502::I_TSX()
{
    X = SP;
    setFlag(Z, X == 0);
    setFlag(N, X & 0x80);
}

void CPU6502::I_TXA()
{
    A = X;
    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::I_TXS()
{
    SP = X;
}

void CPU6502::I_TYA()
{
    A = Y;
    setFlag(Z, A == 0);
    setFlag(N, A & 0x80);
}

void CPU6502::clock()
{
    if (cycles == 0) {
        uint8_t opcode = fetch();
        switch (opcode) {
            case 0x69: I_ADC(immediate());       cycles = 2; break;
            case 0x65: I_ADC(read(zeroPage()));  cycles = 3; break;
            case 0x75: I_ADC(read(zeroPageX())); cycles = 4; break;
            case 0x6D: I_ADC(read(absolute()));  cycles = 4; break;
            case 0x7D: I_ADC(read(absoluteX())); cycles = 4; break;
            case 0x79: I_ADC(read(absoluteY())); cycles = 4; break;
            case 0x61: I_ADC(read(indirectX())); cycles = 6; break;
            case 0x71: I_ADC(read(indirectY())); cycles = 5; break;
            case 0x29: I_AND(immediate());       cycles = 2; break;
            case 0x25: I_AND(read(zeroPage()));  cycles = 3; break;
            case 0x35: I_AND(read(zeroPageX())); cycles = 4; break;
            case 0x2D: I_AND(read(absolute()));  cycles = 4; break;
            case 0x3D: I_AND(read(absoluteX())); cycles = 4; break;
            case 0x39: I_AND(read(absoluteY())); cycles = 4; break;
            case 0x21: I_AND(read(indirectX())); cycles = 6; break;
            case 0x31: I_AND(read(indirectY())); cycles = 5; break;
            case 0x0A: I_ASL_ACC();              cycles = 2; break;
            case 0x06: I_ASL(zeroPage());        cycles = 5; break;
            case 0x16: I_ASL(zeroPageX());       cycles = 6; break;
            case 0x0E: I_ASL(absolute());        cycles = 6; break;
            case 0x1E: I_ASL(absoluteX());       cycles = 7; break;
            case 0x90: I_BCC((int8_t)fetch());   cycles = 2; break;
            case 0xB0: I_BCS((int8_t)fetch());   cycles = 2; break;
            case 0xF0: I_BEQ((int8_t)fetch());   cycles = 2; break;
            case 0x24: I_BIT(read(zeroPage()));  cycles = 3; break;
            case 0x2C: I_BIT(read(absolute()));  cycles = 4; break;
            case 0x30: I_BMI((int8_t)fetch());   cycles = 2; break;
            case 0xD0: I_BNE((int8_t)fetch());   cycles = 2; break;
            case 0x10: I_BPL((int8_t)fetch());   cycles = 2; break;
            case 0x00: I_BRK();                  cycles = 7; break;
            case 0x50: I_BVC((int8_t)fetch());   cycles = 2; break;
            case 0x70: I_BVS((int8_t)fetch());   cycles = 2; break;
            case 0x18: I_CLC();                  cycles = 2; break;
            case 0xD8: I_CLD();                  cycles = 2; break;
            case 0x58: I_CLI();                  cycles = 2; break;
            case 0xB8: I_CLV();                  cycles = 2; break;
            case 0xC9: I_CMP(immediate());       cycles = 2; break;
            case 0xC5: I_CMP(read(zeroPage()));  cycles = 3; break;
            case 0xD5: I_CMP(read(zeroPageX())); cycles = 4; break;
            case 0xCD: I_CMP(read(absolute()));  cycles = 4; break;
            case 0xDD: I_CMP(read(absoluteX())); cycles = 4; break;
            case 0xD9: I_CMP(read(absoluteY())); cycles = 4; break;
            case 0xC1: I_CMP(read(indirectX())); cycles = 6; break;
            case 0xD1: I_CMP(read(indirectY())); cycles = 5; break;
            case 0xE0: I_CPX(immediate());       cycles = 2; break;
            case 0xE4: I_CPX(read(zeroPage()));  cycles = 3; break;
            case 0xEC: I_CPX(read(absolute()));  cycles = 4; break;
            case 0xC0: I_CPY(immediate());       cycles = 2; break;
            case 0xC4: I_CPY(read(zeroPage()));  cycles = 3; break;
            case 0xCC: I_CPY(read(absolute()));  cycles = 4; break;
            case 0xC6: I_DEC(zeroPage());        cycles = 5; break;
            case 0xD6: I_DEC(zeroPageX());       cycles = 6; break;
            case 0xCE: I_DEC(absolute());        cycles = 6; break;
            case 0xDE: I_DEC(absoluteX());       cycles = 7; break;
            case 0xCA: I_DEX();                  cycles = 2; break;
            case 0x88: I_DEY();                  cycles = 2; break;
            case 0x49: I_EOR(immediate());       cycles = 2; break;
            case 0x45: I_EOR(read(zeroPage()));  cycles = 3; break;
            case 0x55: I_EOR(read(zeroPageX())); cycles = 4; break;
            case 0x4D: I_EOR(read(absolute()));  cycles = 4; break;
            case 0x5D: I_EOR(read(absoluteX())); cycles = 4; break;
            case 0x59: I_EOR(read(absoluteY())); cycles = 4; break;
            case 0x41: I_EOR(read(indirectX())); cycles = 6; break;
            case 0x51: I_EOR(read(indirectY())); cycles = 5; break;
            case 0xE6: I_INC(zeroPage());        cycles = 5; break;
            case 0xF6: I_INC(zeroPageX());       cycles = 6; break;
            case 0xEE: I_INC(absolute());        cycles = 6; break;
            case 0xFE: I_INC(absoluteX());       cycles = 7; break;
            case 0xE8: I_INX();                  cycles = 2; break;
            case 0xC8: I_INY();                  cycles = 2; break;
            case 0x4C: I_JMP(absolute());        cycles = 3; break;
            case 0x6C: I_JMP(indirect());        cycles = 5; break;
            case 0x20: I_JSR(absolute());        cycles = 6; break;
            case 0xA9: I_LDA(immediate());       cycles = 2; break;
            case 0xA5: I_LDA(read(zeroPage()));  cycles = 3; break;
            case 0xB5: I_LDA(read(zeroPageX())); cycles = 4; break;
            case 0xAD: I_LDA(read(absolute()));  cycles = 4; break;
            case 0xBD: I_LDA(read(absoluteX())); cycles = 4; break;
            case 0xB9: I_LDA(read(absoluteY())); cycles = 4; break;
            case 0xA1: I_LDA(read(indirectX())); cycles = 6; break;
            case 0xB1: I_LDA(read(indirectY())); cycles = 5; break;
            case 0xA2: I_LDX(immediate());       cycles = 2; break;
            case 0xA6: I_LDX(read(zeroPage()));  cycles = 3; break;
            case 0xB6: I_LDX(read(zeroPageY())); cycles = 4; break;
            case 0xAE: I_LDX(read(absolute()));  cycles = 4; break;
            case 0xBE: I_LDX(read(absoluteY())); cycles = 4; break;
            case 0xA0: I_LDY(immediate());       cycles = 2; break;
            case 0xA4: I_LDY(read(zeroPage()));  cycles = 3; break;
            case 0xB4: I_LDY(read(zeroPageX())); cycles = 4; break;
            case 0xAC: I_LDY(read(absolute()));  cycles = 4; break;
            case 0xBC: I_LDY(read(absoluteX())); cycles = 4; break;
            case 0x4A: I_LSR_ACC();              cycles = 2; break;
            case 0x46: I_LSR(zeroPage());        cycles = 5; break;
            case 0x56: I_LSR(zeroPageX());       cycles = 6; break;
            case 0x4E: I_LSR(absolute());        cycles = 6; break;
            case 0x5E: I_LSR(absoluteX());       cycles = 7; break;
            case 0xEA: I_NOP();                  cycles = 2; break;
            case 0x09: I_ORA(immediate());       cycles = 2; break;
            case 0x05: I_ORA(read(zeroPage()));  cycles = 3; break;
            case 0x15: I_ORA(read(zeroPageX())); cycles = 4; break;
            case 0x0D: I_ORA(read(absolute()));  cycles = 4; break;
            case 0x1D: I_ORA(read(absoluteX())); cycles = 4; break;
            case 0x19: I_ORA(read(absoluteY())); cycles = 4; break;
            case 0x01: I_ORA(read(indirectX())); cycles = 6; break;
            case 0x11: I_ORA(read(indirectY())); cycles = 5; break;
            case 0x48: I_PHA();                  cycles = 3; break;
            case 0x08: I_PHP();                  cycles = 3; break;
            case 0x68: I_PLA();                  cycles = 4; break;
            case 0x28: I_PLP();                  cycles = 4; break;
            case 0x2A: I_ROL_ACC();              cycles = 2; break;
            case 0x26: I_ROL(zeroPage());        cycles = 5; break;
            case 0x36: I_ROL(zeroPageX());       cycles = 6; break;
            case 0x2E: I_ROL(absolute());        cycles = 6; break;
            case 0x3E: I_ROL(absoluteX());       cycles = 7; break;
            case 0x6A: I_ROR_ACC();              cycles = 2; break;
            case 0x66: I_ROR(zeroPage());        cycles = 5; break;
            case 0x76: I_ROR(zeroPageX());       cycles = 6; break;
            case 0x6E: I_ROR(absolute());        cycles = 6; break;
            case 0x7E: I_ROR(absoluteX());       cycles = 7; break;
            case 0x40: I_RTI();                  cycles = 6; break;
            case 0x60: I_RTS();                  cycles = 6; break;
            case 0xE9: I_SBC(immediate());       cycles = 2; break;
            case 0xE5: I_SBC(read(zeroPage()));  cycles = 3; break;
            case 0xF5: I_SBC(read(zeroPageX())); cycles = 4; break;
            case 0xED: I_SBC(read(absolute()));  cycles = 4; break;
            case 0xFD: I_SBC(read(absoluteX())); cycles = 4; break;
            case 0xF9: I_SBC(read(absoluteY())); cycles = 4; break;
            case 0xE1: I_SBC(read(indirectX())); cycles = 6; break;
            case 0xF1: I_SBC(read(indirectY())); cycles = 5; break;
            case 0x38: I_SEC();                  cycles = 2; break;
            case 0xF8: I_SED();                  cycles = 2; break;
            case 0x78: I_SEI();                  cycles = 2; break;
            case 0x85: I_STA(zeroPage());        cycles = 3; break;
            case 0x95: I_STA(zeroPageX());       cycles = 4; break;
            case 0x8D: I_STA(absolute());        cycles = 4; break;
            case 0x9D: I_STA(absoluteX());       cycles = 5; break;
            case 0x99: I_STA(absoluteY());       cycles = 5; break;
            case 0x81: I_STA(indirectX());       cycles = 6; break;
            case 0x91: I_STA(indirectY());       cycles = 6; break;
            case 0x86: I_STX(zeroPage());        cycles = 3; break;
            case 0x96: I_STX(zeroPageY());       cycles = 4; break;
            case 0x8E: I_STX(absolute());        cycles = 4; break;
            case 0x84: I_STY(zeroPage());        cycles = 3; break;
            case 0x94: I_STY(zeroPageX());       cycles = 4; break;
            case 0x8C: I_STY(absolute());        cycles = 4; break;
            case 0xAA: I_TAX();                  cycles = 2; break;
            case 0xA8: I_TAY();                  cycles = 2; break;
            case 0xBA: I_TSX();                  cycles = 2; break;
            case 0x8A: I_TXA();                  cycles = 2; break;
            case 0x9A: I_TXS();                  cycles = 2; break;
            case 0x98: I_TYA();                  cycles = 2; break;
            // ... all 256 opcodes
            default: break;  // illegal opcodes
        }
    }
    cycles--;
}