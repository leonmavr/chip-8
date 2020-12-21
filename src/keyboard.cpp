#include "keyboard.hpp"
#include <cinttypes>
#include <SDL2/SDL.h>
#include <iostream>


uint8_t Keyboard::getKeyPress() {
	// NOTE: this method assumes that SDL has been initialised externally
	
	const uint8_t* keys = SDL_GetKeyboardState(NULL);
	uint8_t keyPressed = 0x0; // 4-bit Chip8 key
	SDL_Event event;
	int paused = 0;
	
	while (SDL_PollEvent(&event)) {
		// key release - reset the keyboard
		if (event.type == SDL_KEYUP)  {
			auto keyPair = m_keyQwerty2Chip8.find(event.key.keysym.sym);
			// then unmapped key - return
			if (keyPair == m_keyQwerty2Chip8.end()) 
				return 0x0;
			keyPressed = keyPair->second;
			for (auto& k: m_keypresses)
				k = 0x0;
		}

		// keypress
		if (event.type == SDL_KEYDOWN)  {
			// pause key (F1)
			if (event.key.keysym.sym == SDLK_F1) {
				while (paused <= 2) {
					SDL_PollEvent(&event);
					paused = ((event.type == SDL_KEYUP) && (paused % 2 == 0))? paused + 1: paused ;
					paused = ((event.type == SDL_KEYDOWN) && (paused % 2 == 1))? paused + 1: paused ;
				}
			}
			// handle gameplay keys - turn on Chip-8's pressed key
			else if (event.key.keysym.sym != SDLK_ESCAPE ) {
				auto keyPair = m_keyQwerty2Chip8.find(event.key.keysym.sym);
				// then unmapped key - return
				if (keyPair == m_keyQwerty2Chip8.end()) 
					return 0x0;
				keyPressed = keyPair->second;
				m_keypresses[keyPressed] = 0x1;
			}
			else // exit on Esc
				std::exit(0);
		}

		// exit on window close
		if (event.type == SDL_QUIT)
				std::exit(0);

	}
	return keyPressed;
}
