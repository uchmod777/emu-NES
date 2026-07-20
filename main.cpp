#include "CPU6502.hpp"
#include "Bus.hpp"
#include "Cartridge.hpp"
#include <fstream>

int main()
{
    Cartridge cart("nestest.nes");
    Bus bus;
    CPU6502 cpu;

    bus.connectCartridge(&cart);
    bus.connectCPU(&cpu);
    cpu.connectBus(&bus);

    cpu.Reset();
    cpu.setPC(0xC000);

    std::ofstream logFile("cpulog.txt");

    for (int i = 0; i < 1000000; i++)
    {
        cpu.clock(logFile);
    }

    logFile.close();
}