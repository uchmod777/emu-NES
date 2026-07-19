#include "Cartridge.hpp"

// CPU Bus Interface
uint8_t Cartridge::cpuRead(uint16_t addr)
{
    // Calculate the zero offset
    uint16_t offset = addr - 0x8000;

    // Determine the mask based on the prgbanks count (16KB mirrored vs 32KB)
    uint16_t mask = (prgBanks > 1) ? 0x7FFF : 0x3FFF;

    // Return the calculated byte
    return prgROM[offset & mask];
}

void Cartridge::cpuWrite(uint16_t addr, uint8_t data)
{    
    if (addr >= 0x6000 && addr <= 0x7FFF && hasBattery)
    {
        prgRAM[(addr - 0x6000) & 0x1FFF] = data;
    }
}

// PPU Bus Interface
uint8_t Cartridge::ppuRead(uint16_t addr)
{
    return chrROM[addr & 0x1FFF];
}

void Cartridge::ppuWrite(uint16_t addr, uint8_t data)
{
    if (chrBanks == 0)
    {
        chrROM[addr & 0x1FFF] = data;
    }
}