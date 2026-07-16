#include "CPU6502.hpp"
#include "Loader.hpp"
#include <fstream>

int main()
{
    CPU6502 cpu;
    LoadROM(cpu, "nestest.nes");
    cpu.Reset();
    cpu.setPC(0xC000);

    std::ofstream logFile("cpulog.txt");

    for (int i = 0; i < 1000000; i++)
    {
        cpu.clock(logFile);
    }

    logFile.close();
}