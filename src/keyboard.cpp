#include "keyboard.hpp"
#include <iostream>
#include <SDL2/SDL.h>


unsigned char Keyboard::getKey() {
	// zero out the kbd?
	for (int i = 0; i < 16; i++)
		;//this->m_kbd[i] = 0;
	const unsigned char* keys = SDL_GetKeyboardState(NULL);
	SDL_Event event;
	
	unsigned keyPressed = 16;
	// This one needs the rendered to be initialised
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYUP )  {
			for (auto& pair: m_keymap) {
				if (pair.first == event.key.keysym.sym)
					keyPressed = pair.second;
			}
			//unsigned keyPressed = (unsigned)chip8Key->second;

			for (int i = 0; i < 16; i++)
				this->m_kbd[i] = 0;
			//this->m_kbd[keyPressed] = 1;
			//printKbd(this->m_kbd);
			return keyPressed;
		}
		if (event.type == SDL_KEYDOWN )  {
			for (auto& pair: m_keymap) {
				if (pair.first == event.key.keysym.sym)
					keyPressed = pair.second;
			}

			for (int i = 0; i < 16; i++)
				this->m_kbd[i] = 1;
			this->m_kbd[keyPressed] = 0;
			return keyPressed;
		}
	}
	return 0x0;
}
