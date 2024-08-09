#include <memory>
#include "chip8_tester.hpp"
#include "chip8.hpp"

int main(int argc, char** argv) {
    auto ch8 = std::make_unique<Chip8>();
    auto tester = std::make_unique<Chip8Tester>();
    ch8->LoadRom("./roms/ibm_logo.ch8");
    ch8->Run(10);
    tester->foo(*ch8); 
    return 0;
}
