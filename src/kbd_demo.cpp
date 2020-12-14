#include "keyboard.hpp"
#include <memory>
#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
	auto kbd = std::make_unique<Keyboard>();
	for (int i =0; i < 1000; i++)
			kbd->getKey();
	return 0;
}
