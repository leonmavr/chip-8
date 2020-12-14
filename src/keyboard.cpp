#include "keyboard.hpp"
#include <iostream>
#include <SDL2/SDL.h>


bool Keyboard::getKey() {
	// This will not be needed when it's inherited. TODO: remove
	SDL_Init(SDL_INIT_EVERYTHING);

	const unsigned char* keys = SDL_GetKeyboardState(NULL);
	SDL_Event event;

	// This one needs the rendered to be initialised
	while (SDL_PollEvent(&event)) {
		std::cout << "event: " << event.type << std::endl;
		if (event.type == SDL_KEYUP) 
			std::cout << "key pressed!\n";
		if (event.type == SDL_KEYDOWN) 
			std::cout << "key released!\n";
	}
}
