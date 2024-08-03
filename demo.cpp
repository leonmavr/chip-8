#include <memory>
#include <string>
#include "chip8.hpp"

int main(int argc, char *argv[])
{
    auto ch8 = std::make_unique<Chip8>();
    ch8->LoadRom(argv[1]);
    ch8->Run();
    return 0;
}
