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
                    // ... all 256 opcodes
                    default: break;  // illegal opcodes
                }
            }
            cycles--;
        }
};