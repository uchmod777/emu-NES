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
    uint64_t totalCycles = 0;

    for (int i = 0; i < 10000; i++)
    {
        cpu.LogCPU(logFile, totalCycles);
        cpu.clock();
        totalCycles++;
    }

    logFile.close();
}