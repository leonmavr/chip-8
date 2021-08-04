#include <memory>
#include <string>
#include "display.hpp" 
#include "chip8.hpp"

int main(int argc, char *argv[])
{
    // test the machine
    auto ch8 = std::make_unique<Chip8>("chip8.ini");
    ch8->loadRom(argv[1]);
    ch8->run();
    return 0;
}
