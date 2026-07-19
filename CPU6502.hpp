#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>

class CPU6502 
{
    public:
        void setPC(uint16_t addr) { PC = addr; }
        uint8_t getCycles() { return cycles; }
        void NMI();
        void IRQ();
        void Reset();
        void LoadMemory(uint16_t addr, uint8_t data);
        void LogCPU(std::ofstream& logFile, uint64_t totalCycles);
        void clock(std::ofstream& logFile); // CPU Fetch, Decode, Execute
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
        // Registers and Cycles
        uint8_t  A;       // Accumulator
        uint8_t  X;       // Index X
        uint8_t  Y;       // Index Y
        uint8_t  SP;      // Stack Pointer (offset from 0x0100)
        uint16_t PC;      // Program Counter
        uint8_t  P;       // Status Flags: N V - B D I Z C
        uint8_t  cycles;  // Remaining cycles for current instruction

        // Opcode lookup table
        // Instruction lengths indexed by opcode
        inline static const uint8_t instructionSizes[256] = {
          //0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
            1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0x00
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0x10
            3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0x20
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0x30
            1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0x40
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0x50
            1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0x60
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0x70
            2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0x80
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0x90
            2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0xA0
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0xB0
            2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0xC0
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0xD0
            2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,  // 0xE0
            2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,  // 0xF0
        };

        // Logging
        uint64_t totalCycles = 0;

        // Memory
        uint8_t  memory[65536];  // 64 KB
        uint8_t  read(uint16_t addr)                { return memory[addr]; }
        void     write(uint16_t addr, uint8_t data) { memory[addr] = data; }
        uint8_t  fetch()                            { return read(PC++); }
        
        // Flags
        enum class Flags { C=0x01, Z=0x02, I=0x04, D=0x08, B=0x10, U=0x20, V=0x40, N=0x80 };
        const uint8_t getFlag(uint8_t f)         { return (P & f) ? 1 : 0;}
        void    setFlag(uint8_t f, bool v) { v ? (P |= f) : (P &= ~f);}

        // Addressing
        uint8_t  immediate()  { return fetch(); }
        uint16_t zeroPage()   { return fetch(); }
        uint16_t zeroPageX()  { return (fetch() + X) & 0xFF; }
        uint16_t zeroPageY()  { return (fetch() + Y) & 0xFF; }
        uint16_t absolute()
        {
            uint8_t low = fetch();
            uint8_t high = fetch();
            return (uint16_t) (high << 8) | low;
        }
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
        uint16_t absoluteX_write()
        {
            uint8_t low = fetch();
            uint8_t high = fetch();
            return ((uint16_t)(high << 8) | low) + X;
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
        uint16_t absoluteY_write()
        {
            uint8_t low = fetch();
            uint8_t high = fetch();
            return ((uint16_t)(high << 8) | low) + Y;
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
        uint16_t indirectY_write()
        {
            uint8_t base = fetch();
            uint16_t addr = read(base) | (read((base + 1) & 0xFF) << 8);
            return addr + Y;
        }

        // Stack
        void push(uint8_t val) { write(0x0100 + SP--, val); }
        uint8_t pop()          { return read(0x0100 + ++SP); }
};