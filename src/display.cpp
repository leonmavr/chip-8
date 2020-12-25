#include <SDL2/SDL.h>
#include <memory>
#include "display.hpp" 
#include <iostream> // for screendump
#include <fstream> // for screendump
#include <string> // to_string()


void Display::init() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
	SDL_Init(SDL_INIT_EVERYTHING);
	m_window = SDL_CreateWindow("Display", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, m_w, m_h, SDL_WINDOW_SHOWN);
	m_renderer = SDL_CreateRenderer(m_window, -1, 0);

	Display::cls();
    //Pause for 100 ms
    SDL_Delay(100);
	
	m_debug = false;
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


void Display::drawPixelXY(unsigned x, unsigned y, unsigned colour) {
	// define the pixel to draw
	SDL_Rect pixel;
	// The original Chip-8 display was 64x32 - scale this up to show it properly
	unsigned scaleX = static_cast<int>(m_w / 64);
	unsigned scaleY = static_cast<int>(m_h / 32);
	pixel.x = x * scaleX;
	pixel.y = y * scaleY;
	pixel.w = scaleX;
	pixel.h = scaleY;

	if (colour == 1)
		SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
	else
		SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(m_renderer, &pixel);
}


const std::string Display::renderAll(unsigned char(&array2D)[32][64]) {
	for (unsigned row = 0; row < 32; row++) {
		for (unsigned col = 0; col < 64; col++) {
			drawPixelXY(col, row, static_cast<unsigned>(array2D[row][col] != 0));
		}
	}
	SDL_RenderPresent(m_renderer); // copy to screen
	// delay (in ms) is REQUIRED or the screen will glitch out
    SDL_Delay(5);

	const std::string mystr = "";
	if (!m_debug)
		return mystr;
	else
		screendump();
}


void Display::cls() {
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



// This method is used for unit testing. It's not needed by Chip-8 itself.
const std::string Display::screendump(const std::string outFile) {
	std::ofstream ofs;
	ofs.open(outFile, std::ofstream::out | std::ofstream::trunc);
	for (unsigned row = 0; row < 32; row++) {
		for (unsigned col = 0; col < 64; col++) {
			ofs << std::to_string(m_display[row][col]);
		}
		ofs << "\n";
	}
	return outFile;
}
