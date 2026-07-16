#pragma once

#include "CPU6502.hpp"
#include <fstream>

void LoadROM(CPU6502& cpu, const std::string& path);