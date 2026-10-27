#include <iostream>
#include <memory>
#include <string>
#include "chip8.hpp"

namespace {

void PrintUsage(const char* program) {
    std::cerr << "usage: " << program << " [-s|--sound] path/to/rom.ch8\n";
}

} // namespace

int main(int argc, char** argv) {
    bool sound = false;
    const char* rom = nullptr;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-s" || arg == "--sound") {
            sound = true;
        } else if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        } else if (rom == nullptr) {
            rom = argv[i];
        } else {
            PrintUsage(argv[0]);
            return 1;
        }
    }

    if (rom == nullptr) {
        PrintUsage(argv[0]);
        return 1;
    }

    auto ch8 = std::make_unique<Chip8>();
    ch8->LoadRom(rom);
    if (sound)
        ch8->StartSound();
    ch8->Run();
    return 0;
}
