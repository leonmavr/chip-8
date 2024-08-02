#include <memory>
#include <string>
#include "chip8.hpp"

int main(int argc, char *argv[])
{
    // test the machine
    auto ch8 = std::make_unique<Chip8>("chip8.ini");
    ch8->LoadRom(argv[1]);
    ch8->Run();
    return 0;
}
