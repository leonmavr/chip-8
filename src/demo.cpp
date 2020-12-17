#include "display.hpp" 
#include "chip8.hpp"
#include <memory>

int main(int argc, char *argv[])
{
		// test the machine
	auto ch8 = std::make_unique<Chip8>(true);
	ch8->loadRom("roms/tetris.c8");
	ch8->run();
	return 0;
}
