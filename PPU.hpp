#pragma once
#include <cstring>
#include "Cartridge.hpp"

class PPU
{
    public:
        // Constructor and destructor
        PPU() : PPUCTRL(0), PPUMASK(0), PPUSTATUS(0), OAMADDR(0),
                v(0), t(0), x(0), w(0),
                bgShiftPatternLo(0), bgShiftPatternHi(0),
                bgShiftAttribLo(0), bgShiftAttribHi(0),
                bgNextTileID(0), bgNextTileAttrib(0),
                bgNextTileLo(0), bgNextTileHi(0),
                secondaryOAM{}, spriteCount(0),
                spriteShiftPatternLo{}, spriteShiftPatternHi{},
                spriteAttributes{}, spriteXPositions{},
                spriteZeroHitPossible(0), spriteZeroBeingRendered(0),
                dataBuffer(0), OAMData(0),
                scanline(-1), cycle(0),
                frameComplete(false), nmiOccurred(false),
                cart(nullptr)
        {
            memset(OAM, 0, sizeof(OAM));
            memset(VRAM, 0, sizeof(VRAM));
            memset(palette, 0, sizeof(palette));
            memset(frameBuffer, 0, sizeof(frameBuffer));
        }
        ~PPU() = default;

        // Interface methods
        uint8_t cpuRead(uint16_t addr);
        void    cpuWrite(uint16_t addr, uint8_t data);
        uint8_t ppuRead(uint16_t addr);
        void    ppuWrite(uint16_t addr, uint8_t data);
        void    writeOAM(uint8_t addr, uint8_t data) { OAM[addr] = data; }
        void    connectCartridge(Cartridge* c);
        void    clock();

        // Timing state needed by the Bus/main loop
        bool frameComplete;
        bool nmiOccurred;

        // Access frame data
        const uint8_t* getFrameBuffer() const { return frameBuffer; }
    private:
        // Registers
        uint8_t PPUCTRL;
        uint8_t PPUMASK;
        uint8_t PPUSTATUS;
        uint8_t OAMADDR;

        // Internal memory
        uint8_t OAM[256];
        uint8_t VRAM[2048];
        uint8_t palette[32];
        uint8_t frameBuffer[256 * 240 * 3];

        // Internal address registers
        uint16_t v;
        uint16_t t;
        uint8_t x;
        uint8_t w;

        // Internal data registers
        uint8_t dataBuffer;
        uint8_t OAMData;

        // Background shift registers
        uint16_t bgShiftPatternLo;
        uint16_t bgShiftPatternHi;
        uint16_t bgShiftAttribLo;
        uint16_t bgShiftAttribHi;

        // Background tile fetch latches
        uint8_t bgNextTileID;
        uint8_t bgNextTileAttrib;
        uint8_t bgNextTileLo;
        uint8_t bgNextTileHi;

        // Secondary OAM - holds up to 8 sprites for current scanline
        uint8_t secondaryOAM[32];
        uint8_t spriteCount;

        // Sprite shift registers
        uint8_t spriteShiftPatternLo[8];
        uint8_t spriteShiftPatternHi[8];
        uint8_t spriteAttributes[8];
        uint8_t spriteXPositions[8];

        // Sprite zero hit tracking
        bool spriteZeroHitPossible;
        bool spriteZeroBeingRendered;

        // Timing
        int scanline;
        int cycle;

        // Cartridge
        Cartridge* cart;

        // Mirroring
        uint8_t mirrorMode;  // set from cart

        // NES Palette Table
        static constexpr uint8_t NES_PALETTE[64][3] = {
            {84,  84,  84},  {0,   30,  116}, {8,   16,  144}, {48,  0,   136},
            {68,  0,   100}, {92,  0,   48},  {84,  4,   0},   {60,  24,  0},
            {32,  42,  0},   {8,   58,  0},   {0,   64,  0},   {0,   60,  0},
            {0,   50,  60},  {0,   0,   0},   {0,   0,   0},   {0,   0,   0},
            {152, 150, 152}, {8,   76,  196}, {48,  50,  236}, {92,  30,  228},
            {136, 20,  176}, {160, 20,  100}, {152, 34,  32},  {120, 60,  0},
            {84,  90,  0},   {40,  114, 0},   {8,   124, 0},   {0,   118, 40},
            {0,   102, 120}, {0,   0,   0},   {0,   0,   0},   {0,   0,   0},
            {236, 238, 236}, {76,  154, 236}, {120, 124, 236}, {176, 98,  236},
            {228, 84,  236}, {236, 88,  180}, {236, 106, 100}, {212, 136, 32},
            {160, 170, 0},   {116, 196, 0},   {76,  208, 32},  {56,  204, 108},
            {56,  180, 204}, {60,  60,  60},  {0,   0,   0},   {0,   0,   0},
            {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236},
            {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
            {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180},
            {160, 214, 228}, {160, 162, 160}, {0,   0,   0},   {0,   0,   0}
        };
};