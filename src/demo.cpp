#include "display.hpp" 
#include "chip8.hpp"
#include <memory>

int main(int argc, char *argv[])
{
	// test display
	auto disp = std::make_unique<Display>();
	disp->drawPixel(31,31);
	disp->drawPixel(30,31);
	disp->drawPixel(29,31);
	disp->drawPixel(28,31);
	disp->reset();
	disp->drawPixel(29,31);
	disp->drawPixel(28,31);
	disp->close();
	// test the machine
	auto ch8 = std::make_unique<Chip8>();
	ch8->loadRom("../roms/tetris.c8");
	ch8->run();
	return 0;
}
