#include "PPU.hpp"
#include <cstdint>

uint8_t PPU::ppuRead(uint16_t addr)
{
    uint8_t data = 0x00;
    addr &= 0x3FFF;

    if (addr <= 0x1FFF)
    {
        data = cart->ppuRead(addr);
    }
    else if (addr >= 0x2000 && addr <= 0x3EFF)
    {
        // Nametable - mirror adddress into 2KB VRAM
        addr &= 0x0FFF;
        if (mirrorMode == 0)  // horizontal mirroring
        {
            if (addr <= 0x03FF) data = VRAM[addr & 0x03FF];
            else if (addr <= 0x07FF) data = VRAM[addr & 0x03FF];
            else if (addr <= 0x0BFF) data = VRAM[0x0400 + (addr & 0x03FF)];
            else data = VRAM[0x0400 + (addr & 0x03FF)];
        }
        else  // vertical mirroring
        {
            data = VRAM[addr & 0x07FF];
        }
    }
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
    {
        // Palette RAM - 32 bytes mirrored
        addr &= 0x001F;
        if (addr == 0x0010) addr = 0x0000;
        else if (addr == 0x0014) addr = 0x0004;
        else if (addr == 0x0018) addr = 0x0008;
        else if (addr == 0x001C) addr = 0x000C;
        data = palette[addr];
    }
    return data;
}

void PPU::ppuWrite(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF)
    {
        // Pattern table - write to cartridge CHR-RAM
        cart->ppuWrite(addr, data);
    }
    else if (addr >= 0x2000 && addr <= 0x3EFF)
    {
        // Nametable - mirror address into 2KB VRAM
        addr &= 0x0FFF;
        if (mirrorMode == 0)  // horizontal mirroring
        {
            if (addr <= 0x03FF) VRAM[addr & 0x03FF] = data;
            else if (addr <= 0x07FF) VRAM[addr & 0x03FF] = data;
            else if (addr <= 0x0BFF) VRAM[0x0400 + (addr & 0x03FF)] = data;
            else VRAM[0x0400 + (addr & 0x03FF)] = data;
        }
        else  // vertical mirroring
        {
            VRAM[addr & 0x07FF] = data;
        }
    }
    else if (addr >= 0x3F00 && addr <= 0x3FFF)
    {
        // Palette RAM - 32 bytes mirrored
        addr &= 0x001F;
        // Mirror sprite palette backgrounds to background palette
        if (addr == 0x0010) addr = 0x0000;
        else if (addr == 0x0014) addr = 0x0004;
        else if (addr == 0x0018) addr = 0x0008;
        else if (addr == 0x001C) addr = 0x000C;
        palette[addr] = data;
    }
}

uint8_t PPU::cpuRead(uint16_t addr)
{
    uint8_t reg = addr & 0x0007;
    uint8_t data = 0x00;

    switch(reg)
    {
        case 0x0002:  // PPUSTATUS
            data = PPUSTATUS;
            PPUSTATUS &= ~0X80;  // clear VBlank flag
            w = 0;  // reset write toggle
            break;
        case 0x0004:  // OAMDATA
            data = OAM[OAMADDR];
            break;
        case 0x0007:  // PPUDATA
            data = dataBuffer;
            dataBuffer = ppuRead(v);

            // Palette reads return immediately without buffering
            if (v >= 0x3F00)
            {
                data = dataBuffer;
            }

            // Increment v by 1 or 32 depending on PPUCTRL bit 2
            v += (PPUCTRL & 0x04) ? 32 : 1;
            break;
        default:
            break;
    }

    return data;
}

void PPU::cpuWrite(uint16_t addr, uint8_t data)
{
    uint8_t reg = addr & 0x0007;

    switch(reg)
    {
        case 0x0000:  // PPUCTRL
            PPUCTRL = data;
            // bits 0-1 of PPUCTRL go into bits 10-11 of t (nametable select)
            t = (t & 0xF3FF) | ((uint16_t)(data & 0x03) << 10);
            break;
        case 0x0001:  // PPUMASK
            PPUMASK = data;
            break;
        case 0x0003:  // OAMADDR
            OAMADDR = data;
            break;
        case 0x0004:  // OAMDATA
            OAM[OAMADDR] = data;
            OAMADDR++;
            break;
        case 0x0005:  // PPUSCROLL
            if (w == 0)
            {
                // First write - X Scroll
                x = data & 0x07;  // fine X (bits 0-2)
                t = (t & 0xFFE0) | (data >> 3);  // coarse X into t bits 0-4
                w = 1;
            }
            else
            {
                // Second write - Y scroll
                t = (t & 0x8FFF) | ((uint16_t)(data & 0x07) << 12);  // fine Y into t bits 12-14
                t = (t & 0xFC1F) | ((uint16_t)(data >> 3) << 5);  // coarse Y into t bits 5-9
                w = 0;
            }
            break;
        case 0x0006:  // PPUADDR
            if (w == 0)
            {
                // First write - high byte
                t = (t & 0x00FF) | ((uint16_t)(data & 0x3F) << 8);
                w = 1;
            }
            else
            {
                // Second write - low byte
                t = (t & 0xFF00) | data;
                v = t;  // copy t to v on second write
                w = 0;
            }
            break;
        case 0x0007:  // PPUDATA
            ppuWrite(v, data);
            v += (PPUCTRL & 0x04) ? 32 : 1;
            break;
        default:
            break;
    }
}

void PPU::connectCartridge(Cartridge* c)
{
    cart = c;
    mirrorMode = cart->getMirrorMode();
}

void PPU::clock()
{
    // Visible scanlines
    if (scanline >= -1 && scanline <= 239)
    {
        // Background rendering
        if (scanline == -1 && cycle == 1)
        {
            PPUSTATUS &= ~0x80;
            PPUSTATUS &= ~0x40;
            PPUSTATUS &= ~0x20;
        }

        if ((cycle >= 2 && cycle <= 257) || (cycle >= 321 && cycle <= 337))
        {
            // Shift registers advance every cycle
            if (PPUMASK & 0x08)
            {
                bgShiftPatternLo <<= 1;
                bgShiftPatternHi <<= 1;
                bgShiftAttribLo  <<= 1;
                bgShiftAttribHi  <<= 1;
            }

            switch ((cycle - 1) % 8)
            {
                case 0:  // Load shift registers with latched data
                    bgShiftPatternLo = (bgShiftPatternLo & 0xFF00) | bgNextTileLo;
                    bgShiftPatternHi = (bgShiftPatternHi & 0xFF00) | bgNextTileHi;
                    bgShiftAttribLo  = (bgShiftAttribLo  & 0xFF00) | ((bgNextTileAttrib & 0x01) ? 0xFF : 0x00);
                    bgShiftAttribHi  = (bgShiftAttribHi  & 0xFF00) | ((bgNextTileAttrib & 0x02) ? 0xFF : 0x00);
                    break;
                case 1:  // Fetch nametable byte
                    bgNextTileID = ppuRead(0x2000 | (v & 0x0FFF));
                    break;
                case 3:  // Fetch attribute byte
                    {
                        uint16_t attribAddr = 0x23C0
                        | (v & 0x0C00)
                        | ((v >> 4) & 0x38)
                        | ((v >> 2) & 0x07);
                        bgNextTileAttrib = ppuRead(attribAddr);

                        if (v & 0x0040) bgNextTileAttrib >>= 4;
                        if (v & 0x0002) bgNextTileAttrib >>= 2;
                        bgNextTileAttrib &= 0x03;
                        break;
                    }
                case 5:  // Fetch pattern table low byte
                    {
                        uint16_t patternAddr = ((uint16_t)(PPUCTRL & 0x10) << 8)
                        + ((uint16_t)bgNextTileID << 4)
                        + ((v >> 12) & 0x07);
                        bgNextTileLo = ppuRead(patternAddr);
                        break;
                    }
                case 7:  // Fetch pattern table high byte
                    {
                        uint16_t patternAddr = ((uint16_t)(PPUCTRL & 0x10) << 8)
                        + ((uint16_t)bgNextTileID << 4)
                        + ((v >> 12) & 0x07) + 8;
                        bgNextTileHi = ppuRead(patternAddr);
                        
                        // Increment coarse X after fetching tile data
                        if (PPUMASK & 0x08)
                        {
                            if ((v & 0x001F) == 31)
                            {
                                v &= ~0x001F;
                                v ^= 0x0400;
                            }
                            else
                            {
                                v++;
                            }
                        }
                        break;
                    }
            }
        }

        // Increment fine Y at cycle 256
        if (cycle == 256)
        {
            if (PPUMASK & 0x08)
            {
                if ((v & 0x7000) != 0x7000)
                {
                    v += 0x1000;
                }
                else
                {
                    v &= ~0x7000;
                    uint16_t coarseY = (v & 0x03E0) >> 5;
                    if (coarseY == 29)
                    {
                        coarseY = 0;
                        v ^= 0x0800;
                    }
                    else if (coarseY == 31) 
                    {
                        coarseY = 0;
                    }
                    else
                    {
                        coarseY++;
                    }
                    v = (v & ~0x03E0) | (coarseY << 5);
                }
            }
        }

        // Copy horizontal bits from t to v at cycle 257
        if (cycle == 257)
        {
            if (PPUMASK & 0x08)
            {
                v = (v & 0xFBE0) | (t & 0x041F);
            }
        }

        if (cycle == 257 && scanline >= 0 && scanline <= 239)
        {
            memset(secondaryOAM, 0xFF, sizeof(secondaryOAM));
            spriteCount = 0;
            spriteZeroHitPossible = false;

            for (int i = 0; i < 64 && spriteCount < 8; i++)
            {
                int spriteY = OAM[i * 4 + 0];
                int spriteHeight = (PPUCTRL & 0x20) ? 16 : 8;
                int diff = scanline - spriteY;

                if (diff >= 0 && diff < spriteHeight)
                {
                    if (i == 0)
                    {
                        spriteZeroHitPossible = true;
                    }

                    memcpy(&secondaryOAM[spriteCount * 4], &OAM[i * 4], 4);
                    spriteCount++;
                }
            }

            if (spriteCount > 8)
            {
                PPUSTATUS |= 0x20;
            }
        }

        // Copy vertical bits from t to v during pre-render scanline
        if (scanline == -1 && cycle >= 280 && cycle <= 304)
        {
            if (PPUMASK & 0x08)
            {
                v = (v & 0x841F) | (t & 0x7BE0);
            }
        }
    }

    // VBlank start
    if (scanline == 241 && cycle == 1)
    {
        PPUSTATUS |= 0x80;  // set VBlank flag
        if (PPUCTRL & 0x80)  // NMI enable
        {
            nmiOccurred = true;
        }
    }

    // Output pixel
    uint8_t bgPixel   = 0x00;
    uint8_t bgPalette = 0x00;

    if (PPUMASK & 0x08)
    {
        uint16_t bitMux = 0x8000 >> x;
        uint8_t p0 = (bgShiftPatternLo & bitMux) > 0;
        uint8_t p1 = (bgShiftPatternHi & bitMux) > 0;
        bgPixel = (p1 << 1) | p0;

        uint8_t pal0 = (bgShiftAttribLo & bitMux) > 0;
        uint8_t pal1 = (bgShiftAttribHi & bitMux) > 0;
        bgPalette = (pal1 << 1) | pal0;
    }

    // Sprite pixel
    uint8_t spritePixel = 0x00;
    uint8_t spritePalette = 0x00;
    bool spritePriority = false;

    if (PPUMASK & 0x10)
    {
        spriteZeroBeingRendered = false;

        for (int i = 0; i < spriteCount; i++)
        {
            if (spriteXPositions[i] == 0)
            {
                uint8_t lo = (spriteShiftPatternLo[i] & 0x80) > 0;
                uint8_t hi = (spriteShiftPatternHi[i] & 0x80) > 0;
                spritePixel = (hi << 1) | lo;
                spritePalette = (spriteAttributes[i] & 0x03) + 0x04;
                spritePriority = !(spriteAttributes[i] & 0x20);

                if (spritePixel != 0)
                {
                    if (i == 0)
                    {
                        spriteZeroBeingRendered = true;
                    }
                    break;
                }
            }
        }
    }

    // Shift sprite registers
    for (int i = 0; i < spriteCount; i++)
    {
        if (spriteXPositions[i] > 0)
        {
            spriteXPositions[i]--;
        }
        else
        {
            spriteShiftPatternLo[i] <<= 1;
            spriteShiftPatternHi[i] <<= 1;
        }
    }

    // Combine background and sprite pixels
    uint8_t finalPixel = 0x00;
    uint8_t finalPalette = 0x00;

    if (bgPixel == 0 && spritePixel == 0)
    {
        finalPixel = 0x00;
        finalPalette = 0x00;
    }
    else if (bgPixel == 0 && spritePixel > 0)
    {
        finalPixel = spritePixel;
        finalPalette = spritePalette;
    }
    else if (bgPixel > 0 && spritePixel == 0)
    {
        finalPixel = bgPixel;
        finalPalette = bgPalette;
    }
    else
    {
        // Both pixels visible - priority determines winner
        if (spritePriority)
        {
            finalPixel = spritePixel;
            finalPalette = spritePalette;
        }
        else
        {
            finalPixel = bgPixel;
            finalPalette = bgPalette;
        }

        // Sprite zero hit detection
        if (spriteZeroHitPossible && spriteZeroBeingRendered)
        {
            if (PPUMASK & 0x08 && PPUMASK & 0x10)
            {
                if (!(PPUMASK & 0x02) && !(PPUMASK & 0x04))
                {
                    if (cycle >= 9 && cycle <= 257)
                    {
                        PPUSTATUS |= 0x40;
                    }
                }
                else
                {
                    if (cycle >= 1 && cycle <= 257)
                    {
                        PPUSTATUS |= 0x40;
                    }
                }
            }
        }
    }

    // Write final pixel to framebuffer
    uint8_t paletteIndex = ppuRead(0x3F00 + (finalPalette << 2) + finalPixel) & 0x3F;

    if (cycle >= 1 && cycle <= 256 && scanline >= 0 && scanline <= 239)
    {
        int pixelIndex = (scanline * 256 + (cycle - 1)) * 3;
        frameBuffer[pixelIndex + 0] = NES_PALETTE[paletteIndex][0];
        frameBuffer[pixelIndex + 1] = NES_PALETTE[paletteIndex][1];
        frameBuffer[pixelIndex + 2] = NES_PALETTE[paletteIndex][2];
    }

    if (cycle == 340 && scanline >= 0 && scanline <= 239)
    {
        for (int i = 0; i < spriteCount; i++)
        {
            uint8_t spriteY = secondaryOAM[i * 4 + 0];
            uint8_t tileIndex = secondaryOAM[i * 4 + 1];
            uint8_t attributes = secondaryOAM[i * 4 + 2];
            uint8_t spriteX = secondaryOAM[i * 4 + 3];

            spriteAttributes[i] = attributes;
            spriteXPositions[i] = spriteX;

            uint8_t row = scanline - spriteY;

            // Vertical flip
            if (attributes & 0x80)
            {
                row = 7 - row;
            }

            uint16_t patternAddr;
            if (!(PPUCTRL & 0x20))  // 8x8 sprites
            {
                patternAddr = ((uint16_t)(PPUCTRL & 0x08) << 9)
                            + ((uint16_t)tileIndex << 4)
                            + row;
            }
            else  // 8x16 sprites
            {
                if (row >= 8)
                {
                    row += 8;
                }
                patternAddr = ((uint16_t)(tileIndex & 0x01) << 12)
                            + ((uint16_t)(tileIndex & 0xFE) << 4)
                            + row;
            }

            uint8_t lo = ppuRead(patternAddr);
            uint8_t hi = ppuRead(patternAddr + 8);

            // Horizontal flip
            if (attributes & 0x40)
            {
                auto flipByte = [](uint8_t b) {
                    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
                    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
                    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
                    return b;
                };
                lo = flipByte(lo);
                hi = flipByte(hi);
            }

            spriteShiftPatternLo[i] = lo;
            spriteShiftPatternHi[i] = hi;
        }
    }

    // Advance cycle and scanline counters
    cycle++;
    if (cycle >= 341)
    {
        cycle = 0;
        scanline++;
        if (scanline >= 261)
        {
            scanline = -1;
            frameComplete = true;
        }
    }
}