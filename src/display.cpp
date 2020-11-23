#include <SDL2/SDL.h>
#include <memory>
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


void Display::drawBlock(unsigned x, unsigned y) {
	// Ensure it's in Chip-8's display
	assert((0 <= x) && (x < 32) && (0 <= y) && (y < 64));

	// define the block to draw ("pixel")
	SDL_Rect block;
	unsigned scaleX = static_cast<int>(m_w / 32);
	unsigned scaleY = static_cast<int>(m_h / 64);
	block.x = x * scaleX;
	block.y = y * scaleY;
	// The original Chip-8 display was 32x64
	block.w = scaleX;
	block.h = scaleY;

	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255); // the rect color (solid red)
	SDL_RenderFillRect(m_renderer, &block);
	SDL_RenderPresent(m_renderer); // copy to screen

    SDL_Delay(5);
}


void Display::reset() {
	// r, g, b, a
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    //Clear the renderer with the draw color
    SDL_RenderClear(m_renderer);
    SDL_Delay(5);
}
