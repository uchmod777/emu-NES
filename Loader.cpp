#include "Loader.hpp"

void LoadROM(CPU6502& cpu, const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    // Skip 16-byte iNES header
    file.seekg(16);
    // Read 16KB PRG-ROM into 0x8000 and mirror at 0xC000
    for (int i = 0; i < 0x4000; i++)
    {
        uint8_t byte;
        file.read((char*)&byte, 1);
        cpu.LoadMemory(0x8000 + i, byte);
        cpu.LoadMemory(0xC000 + i, byte);
    }
}