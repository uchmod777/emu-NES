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