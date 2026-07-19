#include "Bus.hpp"

void Bus::connectCPU(CPU6502* c)
{
    cpu = c;
}

void Bus::connectPPU(PPU* p)
{
    ppu = p;
}

void Bus::connectCartridge(Cartridge* c)
{
    cart = c;
}

void Bus::setControllerState(uint8_t controller, uint8_t state)
{
    controllerState[controller] = state;
}

uint8_t Bus::read(uint16_t addr)
{
    // CPU RAM - 2KB mirrored across 0x0000-0x1FFF
    if (addr <= 0x1FFF)
    {
        return ram[addr & 0x07FF];
    }

    // PPU registers - 8 bytes mirrored across 0x2000-0x3FFF
    if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        return 0x00;  // stub until PPU is built
    }

    // Controller 1
    if (addr == 0x4016)
    {
        uint8_t bit = (controllerShift[0] & 0x80) ? 1 : 0;
        controllerShift[0] <<= 1;
        return bit;
    }

    // Controller 2
    if (addr == 0x4017)
    {
        uint8_t bit = (controllerShift[1] & 0x80) ? 1 : 0;
        controllerShift[1] <<= 1;
        return bit;
    }

    // Cartridge space
    if (addr >= 0x4020)
    {
        return cart->cpuRead(addr);
    }

    return 0x00;
}

void Bus::write(uint16_t addr, uint8_t data)
{
    // CPU RAM - 2KB mirrored across 0x0000-0x1FFF
    if (addr <= 0x1FFF)
    {
        ram[addr & 0x07FF] = data;
        return;
    }

    // PPU registers - 8 bytes mirrored across 0x2000-0x3FFF
    if (addr >= 0x2000 && addr <= 0x3FFF)
    {
        return;
    }

    // Controller strobe
    if (addr == 0x4016)
    {
        if (data & 0x01)
        {
            // strobe on - reload shift registers from current state
            controllerShift[0] = controllerState[0];
            controllerShift[1] = controllerState[1];
        }
        return;
    }

    // Cartridge space
    if (addr >= 0x4020)
    {
        cart->cpuWrite(addr, data);
    }
}