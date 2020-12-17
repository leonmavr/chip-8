#include "keyboard.hpp"
#include <cinttypes>
#include <SDL2/SDL.h>


uint8_t Keyboard::getKeyPress() {
	// this method assumes that SDL has been initialised externally
	
	const uint8_t* keys = SDL_GetKeyboardState(NULL);
	uint8_t keyPressed = 0x0; // 4-bit Chip8 key
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		// key release
		if (event.type == SDL_KEYUP )  {
			keyPressed = m_keyQwerty2Chip8.find(event.key.keysym.sym)->second;
			for (auto& keypress: m_keypresses)
				keypress = 0x0;
			return keyPressed;
		}

		// keypress
		if (event.type == SDL_KEYDOWN )  {
			if (!(event.key.keysym.sym == SDLK_ESCAPE)) {
				keyPressed = m_keyQwerty2Chip8.find(event.key.keysym.sym)->second;
				m_keypresses[keyPressed] = 0x1;
				return keyPressed;
			} else
				std::exit(0);
		}
		// exit on window close or esc key press
		if (event.type == SDL_QUIT)
				std::exit(0);

	}
	return 0x0;
}
