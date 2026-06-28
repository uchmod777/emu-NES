#include <cstdint>

class CPU6502 
{
    public:
        void I_ADC();  // Add with Carry
        void I_AND();  // Bitwise AND
        void I_ASL();  // Arithmetic Shift Left
        void I_BCC();  // Branch if Carry Clear
        void I_BEQ();  // Branch if Equal
        void I_BIT();  // Bit Test
        void I_BMI();  // Branch if Minus
        void I_BNE();  // Branch if Not Equal
        void I_BPL();  // Branch if Plus
        void I_BRK();  // Break (software IRQ - interrupt request)
        void I_BVC();  // Branch if Overflow Clear
        void I_BVS();  // Branch if Overflow Set
        void I_CLC();  // Clear Carry
        void I_CLD();  // Clear Decimal
        void I_CLI();  // Clear Interrupt Disable
        void I_CLV();  // Clear Overflow
        void I_CMP();  // Compare A
        void I_CPX();  // Compare X
        void I_CPY();  // Compare Y
        void I_DEC();  // Decrement Memory
        void I_DEX();  // Decrement X
        void I_DEY();  // Decrement Y
        void I_EOR();  // Bitwise Exclusive OR
        void I_INC();  // Increment Memory
        void I_INX();  // Increment X
        void I_INY();  // Increment Y
        void I_JMP();  // Jump
        void I_JSR();  // Jump to Subroutine
        void I_LDA();  // Load A
        void I_LDX();  // Load X
        void I_LDY();  // Load Y
        void I_LSR();  // Logical Shift Right
        void I_NOP();  // No Operation
        void I_ORA();  // Bitwise OR
        void I_PHA();  // Push A
        void I_PHP();  // Push Processor Status
        void I_PLA();  // Pull A
        void I_PLP();  // Pull Processor Status
        void I_ROL();  // Rotate Left
        void I_ROR();  // Rotate Right
        void I_RTI();  // Return from Interrupt
        void I_RTS();  // Return from Subroutine
        void I_SBC();  // Subtract with Carry
        void I_SEC();  // Set Carry
        void I_SED();  // Set Decimal
        void I_SEI();  // Set Interrupt Disable
        void I_STA();  // Store A
        void I_STX();  // Store X
        void I_STY();  // Store Y
        void I_TAX();  // Transfer A to X
        void I_TAY();  // Tranfter A to Y
        void I_TSX();  // Transfer Stack Pointer to X
        void I_TXA();  // Transfer X to A
        void I_TXS();  // Transfer X to Stack Pointer
        void I_TYA();  // Transfer Y to A
    private:
        uint8_t  A;    // Accumulator
        uint8_t  X;    // Index X
        uint8_t  Y;    // Index Y
        uint8_t  SP;   // Stack Pointer (offset from 0x0100)
        uint16_t PC;   // Program Counter
        uint8_t  P;    // Status Flags: N V - B D I Z C
        uint16_t memory[65536];
        enum Flags { C=0x01, Z=0x02, I=0x04, D=0x08, B=0x10, U=0x20, V=0x40, N=0x80 };
        uint8_t getFlag(uint8_t f)  { return (P & f) ? 1 : 0;}
        void    setFlag(uint8_t f, bool v) { v ? (P |= f) : (P &= ~f);}
};