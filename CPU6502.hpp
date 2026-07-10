#include <cstdint>

class CPU6502 
{
    public:
        void I_ADC(uint8_t operand);  // Add with Carry
        void I_AND(uint8_t operand);   // Bitwise AND
        void I_ASL_ACC();  // Arithmetic Shift Left, Accumulator
        void I_ASL(uint16_t addr);  // Arithmetic Shift Left
        void I_BCC(int8_t offset);  // Branch if Carry Clear
        void I_BCS(int8_t offset);  // Branch if Carry Set
        void I_BEQ(int8_t offset);  // Branch if Equal
        void I_BIT(uint8_t operand);  // Bit Test
        void I_BMI(int8_t offset);  // Branch if Minus
        void I_BNE(int8_t offset);  // Branch if Not Equal
        void I_BPL(int8_t offset);  // Branch if Plus
        void I_BRK();  // Break (software IRQ - interrupt request)
        void I_BVC(int8_t offset);  // Branch if Overflow Clear
        void I_BVS(int8_t offset);  // Branch if Overflow Set
        void I_CLC();  // Clear Carry
        void I_CLD();  // Clear Decimal
        void I_CLI();  // Clear Interrupt Disable
        void I_CLV();  // Clear Overflow
        void I_CMP(uint8_t operand);  // Compare A
        void I_CPX(uint8_t operand);  // Compare X
        void I_CPY(uint8_t operand);  // Compare Y
        void I_DEC(uint16_t addr);  // Decrement Memory
        void I_DEX();  // Decrement X
        void I_DEY();  // Decrement Y
        void I_EOR(uint8_t operand);  // Bitwise Exclusive OR
        void I_INC(uint16_t addr);  // Increment Memory
        void I_INX();  // Increment X
        void I_INY();  // Increment Y
        void I_JMP(uint16_t addr);  // Jump
        void I_JSR(uint16_t addr);  // Jump to Subroutine
        void I_LDA(uint8_t operand);  // Load A
        void I_LDX(uint8_t operand);  // Load X
        void I_LDY(uint8_t operand);  // Load Y
        void I_LSR(uint16_t addr);  // Logical Shift Right
        void I_LSR_ACC();  // Logical Shift Right Accumulator
        void I_NOP();  // No Operation
        void I_ORA(uint8_t operand);  // Bitwise OR
        void I_PHA();  // Push A
        void I_PHP();  // Push Processor Status
        void I_PLA();  // Pull A
        void I_PLP();  // Pull Processor Status
        void I_ROL(uint16_t addr);  // Rotate Left
        void I_ROL_ACC();  // Rotate left accumulator
        void I_ROR(uint16_t addr);  // Rotate Right
        void I_ROR_ACC();  // Rotate right accumulator
        void I_RTI();  // Return from Interrupt
        void I_RTS();  // Return from Subroutine
        void I_SBC(uint8_t operand);  // Subtract with Carry
        void I_SEC();  // Set Carry
        void I_SED();  // Set Decimal
        void I_SEI();  // Set Interrupt Disable
        void I_STA(uint16_t addr);  // Store A
        void I_STX(uint16_t addr);  // Store X
        void I_STY(uint16_t addr);  // Store Y
        void I_TAX();  // Transfer A to X
        void I_TAY();  // Tranfter A to Y
        void I_TSX();  // Transfer Stack Pointer to X
        void I_TXA();  // Transfer X to A
        void I_TXS();  // Transfer X to Stack Pointer
        void I_TYA();  // Transfer Y to A
    private:
        // Components
        uint8_t  A;       // Accumulator
        uint8_t  X;       // Index X
        uint8_t  Y;       // Index Y
        uint8_t  SP;      // Stack Pointer (offset from 0x0100)
        uint16_t PC;      // Program Counter
        uint8_t  P;       // Status Flags: N V - B D I Z C
        uint8_t  cycles;  // Remaining cycles for current instruction

        // Memory
        uint8_t  memory[65536];
        uint8_t  read(uint16_t addr)                { return memory[addr]; }
        void     write(uint16_t addr, uint8_t data) { memory[addr] = data; }
        uint8_t  fetch()                            { return read(PC++); }
        
        // Flags
        enum Flags { C=0x01, Z=0x02, I=0x04, D=0x08, B=0x10, U=0x20, V=0x40, N=0x80 };
        uint8_t getFlag(uint8_t f)         { return (P & f) ? 1 : 0;}
        void    setFlag(uint8_t f, bool v) { v ? (P |= f) : (P &= ~f);}

        // Addressing
        uint8_t  immediate()  { return fetch(); }
        uint16_t zeroPage()   { return fetch(); }
        uint16_t zeroPageX()  { return (fetch() + X) & 0xFF; }
        uint16_t zeroPageY()  { return (fetch() + Y) & 0xFF; }
        uint16_t absolute()   { uint16_t lo = fetch(); return lo | (fetch() << 8); }
        uint16_t absoluteX() 
        {
            uint16_t base = absolute();
            uint16_t addr = base + X;
            if ((base & 0xFF00) != (addr & 0xFF00))
            {
                cycles++;  // Page boundary penalty
            }
            return addr;
        }
        uint16_t absoluteY()
        {
            uint16_t base = absolute();
            uint16_t addr = base + Y;
            if ((base & 0xFF00) != (addr & 0xFF00))
            {
                cycles++;  // Page boundary penalty
            }
            return addr;
        }
        uint16_t indirect() {
            uint16_t ptr = absolute();

            if ((ptr & 0x00FF) == 0x00FF) 
            {
                return read(ptr) | (read(ptr & 0xFF00) << 8);
            }

            return read(ptr) | (read(ptr + 1) << 8);
        }
        uint16_t indirectX()  { uint8_t base = (fetch() + X) & 0xFF;
                                return read(base) | (read((base+1) & 0xFF) << 8); }
        uint16_t indirectY()
        {
            uint8_t base = fetch();
            uint16_t addr = read(base) | (read((base + 1) & 0xFF) << 8);
            uint16_t result = addr + Y;
            if ((addr & 0xFF00) != (result & 0xFF00))
            {
                cycles++;  // Page boundary penalty
            }
            return result;
        }

        // Stack
        void push(uint8_t val) { write(0x0100 + SP--, val); }
        uint8_t pop()          { return read(0x0100 + ++SP); }

        // CPU Fetch, Decode, Execute
        void clock() {
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
                    case 0xE6: I_INC(zeroPage());  cycles = 5; break;
                    case 0xF6: I_INC(zeroPageX()); cycles = 6; break;
                    case 0xEE: I_INC(absolute());  cycles = 6; break;
                    case 0xFE: I_INC(absoluteX()); cycles = 7; break;
                    case 0xE8: I_INX();            cycles = 2; break;
                    case 0xC8: I_INY();            cycles = 2; break;
                    case 0x4C: I_JMP(absolute());  cycles = 3; break;
                    case 0x6C: I_JMP(indirect());  cycles = 5; break;
                    case 0x20: I_JSR(absolute());  cycles = 6; break;
                    case 0xA9: I_LDA(immediate()); cycles = 2; break;
                    case 0xA5: I_LDA(read(zeroPage())); cycles = 3; break;
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
                    // ... all 256 opcodes
                    default: break;  // illegal opcodes
                }
            }
            cycles--;
        }
};