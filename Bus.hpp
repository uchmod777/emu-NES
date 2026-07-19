#include <cstdint>
#include <array>
#include "CPU6502.hpp"
#include "Cartridge.hpp"
#include "PPU.hpp"

class Bus
{
    public:
        uint8_t read(uint16_t addr);
        void    write(uint16_t addr, uint8_t data);
        void    connectCPU(CPU6502* c);
        void    connectPPU(PPU* p);
        void    connectCartridge(Cartridge* c);
        void    setControllerState(uint8_t controller, uint8_t state);
    private:
        CPU6502*    cpu;
        PPU*        ppu;
        Cartridge*  cart;

        std::array<uint8_t, 2048> ram;

        uint8_t controllerState[2];
        uint8_t controllerShift[2];
};