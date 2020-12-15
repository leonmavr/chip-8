#include <SDL2/SDL.h>
#include <memory>
#include <tuple>
#include <iostream>
#include "display.hpp" 
#include <assert.h>


void Display::init() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
	SDL_Init(SDL_INIT_EVERYTHING);
	m_window = SDL_CreateWindow("Display", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, m_w, m_h, SDL_WINDOW_SHOWN);
	m_renderer = SDL_CreateRenderer(m_window, -1, 0);

	Display::reset();
    //Pause for 100 ms
    SDL_Delay(100);
}


void Display::close() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
    //Destroy the class's renderer 
    SDL_DestroyRenderer(m_renderer);
    //Destroy the class's window
    SDL_DestroyWindow(m_window);
    //Close all the systems of SDL initialized at the top
    SDL_Quit();
}


void Display::drawPixelXY(unsigned x, unsigned y, unsigned val) {
	// Ensure it's in Chip-8's display
	//assert((0 <= x) && (x < 32) && (0 <= y) && (y < 64));
	//if (!(0 <= x) || !(x < 32) || !(0 <= y) || !(y < 64)) 
	//	return;

	// define the pixel to draw
	SDL_Rect pixel;
	unsigned scaleX = static_cast<int>(m_w / 64);
	unsigned scaleY = static_cast<int>(m_h / 32);
	pixel.x = x * scaleX;
	pixel.y = y * scaleY;
	// The original Chip-8 display was 32x64
	pixel.w = scaleX;
	pixel.h = scaleY;

	if (val == 1)
		SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
	else
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(m_renderer, &pixel);
}


void Display::renderAll(unsigned char(&array2D)[32][64]) {
	for (unsigned row = 0; row < 32; row++) {
		for (unsigned col = 0; col < 64; col++) {
			if (array2D[row][col] != 0 )
				drawPixelXY(col, row, 1);
			else
				drawPixelXY(col, row, 0);
		}
	}
	SDL_RenderPresent(m_renderer); // copy to screen
	// delay is REQUIRED or the screen will glitch out
    SDL_Delay(5);
}


void Display::reset() {
	// clear the display array first
	for (int row = 0; row < 32; row++) {
		for (int col= 0; col < 64; col++){
			m_display[row][col] = 0;
			drawPixelXY(col, row, 0);
		}
	}
	// r, g, b, a
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    //Clear the renderer with the draw color
    SDL_RenderClear(m_renderer);
    SDL_Delay(5);
}

static void printKbd(unsigned char * arr) {
	std::cout << "kbd disp: ";
	for (int i = 0; i < 16; i++)
		std::cout <<(int) arr[i] << ", ";
	std::cout << std::endl;
}

unsigned char Display::getKey() {
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
			kp = keyPressed;
			return keyPressed;
		}
		if (event.type == SDL_KEYDOWN )  {
			for (auto& pair: m_keymap) {
				if (pair.first == event.key.keysym.sym)
					keyPressed = pair.second;
			}
			//unsigned keyPressed = (unsigned)chip8Key->second;

			for (int i = 0; i < 16; i++)
				this->m_kbd[i] = 1;
			this->m_kbd[keyPressed] = 0;
			//printKbd(this->m_kbd);
			kp = keyPressed;
			return keyPressed;
		}
	}
	kp = keyPressed;
	return 0x0;
}
