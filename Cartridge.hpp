#include <vector>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

class Cartridge
{
    public:
        struct iNESHeader {
            uint8_t magic[4];
            uint8_t prgROMSize;
            uint8_t chrROMSize;
            uint8_t flags6;
            uint8_t flags7;
            uint8_t prgRamSize;
            uint8_t flags9;
            uint8_t flags10;
            uint8_t padding[5];
        };

        Cartridge(const std::filesystem::path& filePath) : mapperID(0), mirrorMode(0), prgBanks(0), chrBanks(0),
            hasBattery(false), hasTrainer(false)
        {
            std::ifstream file(filePath, std::ios::binary);
            iNESHeader header{};

            if (!file.is_open())
            {
                throw std::runtime_error("Failed to open ROM file: " + filePath.string());
            }

            // Read the 16-byte header
            file.read(reinterpret_cast<char*>(&header), sizeof(iNESHeader));

            // Validate magic number
            if (header.magic[0] != 'N' || header.magic[1] != 'E' ||
                header.magic[2] != 'S' || header.magic[3] != 0x1A)
            {
                throw std::runtime_error("Invalid iNES file format signature.");
            }

            // Extract class members from the header
            prgBanks = header.prgROMSize;
            chrBanks = header.chrROMSize;
            mapperID = (header.flags7 & 0xF0) | (header.flags6 >> 4);
            mirrorMode = header.flags6 & 0x01;
            hasBattery = header.flags6 & 0x02;
            hasTrainer = header.flags6 & 0x04;

            // Check for optional 512-byte trainer flag (Bit 2 of flag6)
            if (hasTrainer)
            {
                file.seekg(512, std::ios::cur);  // Skip trainer data for basic parsing
            }

            if (hasBattery)
            {
                prgRAM.resize(8192);
            }

            // Calculate data block sizes
            size_t prgSize = header.prgROMSize * 16384;  // 16 KiB segments
            size_t chrSize = header.chrROMSize * 8192;  // 8 Kib segments

            // Allocate vectors and read raw byte blocks
            prgROM.resize(prgSize);
            file.read(reinterpret_cast<char*>(prgROM.data()), prgSize);

            if (chrSize > 0)
            {
                chrROM.resize(chrSize);
                file.read(reinterpret_cast<char*>(chrROM.data()), chrSize);
            } else 
            {
                chrROM.resize(8192);  // CHR-RAM
            }

            std::cout << "Successfully loaded ROM.\n"
                      << "PRG ROM Size: " << prgSize / 1024 << " KB\n"
                      << "CHR: " << (chrBanks > 0 ? std::to_string(chrSize / 1024) + " KB ROM" : "8 KB RAM") << "\n";
        }

        // CPU bus interface
        uint8_t cpuRead(uint16_t addr);
        void    cpuWrite(uint16_t addr, uint8_t data);

        // PPU bus interface
        uint8_t ppuRead(uint16_t addr);
        void    ppuWrite(uint16_t addr, uint8_t data);
        
        uint8_t getMirrorMode() const { return mirrorMode; }
    private:
        std::vector<uint8_t> prgROM;
        std::vector<uint8_t> chrROM;
        std::vector<uint8_t> prgRAM;

        uint8_t mapperID;
        uint8_t mirrorMode;
        uint8_t prgBanks;
        uint8_t chrBanks;
        bool    hasBattery;
        bool    hasTrainer;
};