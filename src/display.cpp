#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include "display.hpp" 


void Display::init() {
	// see https://caveofprogramming.com/guest-posts/creating-a-window-with-sdl.html
	SDL_Init(SDL_INIT_EVERYTHING);
	m_window = SDL_CreateWindow("Display", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, m_w, m_h, SDL_WINDOW_SHOWN);
	m_renderer = SDL_CreateRenderer(m_window, -1, 0);

	Display::cls();
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


void Display::drawPixelXY(unsigned x, unsigned y, unsigned colour) {
	// define the pixel to draw
	SDL_Rect pixel;
	// The original Chip-8 display was 64x32 - scale this up to show it properly
	unsigned scaleX = static_cast<unsigned>(m_w / 64);
	unsigned scaleY = static_cast<unsigned>(m_h / 32);
	pixel.x = x * scaleX;
	pixel.y = y * scaleY;
	pixel.w = scaleX;
	pixel.h = scaleY;

	if (colour == 1)
		// renderer, r, g, b, a
		SDL_SetRenderDrawColor(m_renderer, m_colourFg[0], m_colourFg[1], m_colourFg[2], 0xff);
	else
		SDL_SetRenderDrawColor(m_renderer, m_colourBg[0], m_colourBg[1], m_colourBg[2], 0);
	SDL_RenderFillRect(m_renderer, &pixel);
}


void Display::renderAll(unsigned char(&array2D)[32][64]) {
	for (unsigned row = 0; row < 32; row++) {
		for (unsigned col = 0; col < 64; col++) {
			drawPixelXY(col, row, static_cast<unsigned>(array2D[row][col] != 0));
		}
	}
	SDL_RenderPresent(m_renderer); // copy to screen
	// delay (in ms) is REQUIRED or the screen will glitch out
    SDL_Delay(5);
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
	SDL_SetRenderDrawColor(m_renderer, m_colourBg[0], m_colourBg[1], m_colourBg[2], 0);
    //Clear the renderer with the draw color
    SDL_RenderClear(m_renderer);
    SDL_Delay(5);
}


void Display::hex2rgb(std::string strHex, std::vector<uint8_t>& vecrgb) {
	// remove leading # if necessary
	if (strHex.rfind("#", 0) == 0)
		strHex = strHex.substr(1, strHex.length());

	// extract the 3 bytes
	unsigned rgb = std::stoi(strHex, 0, 16);
	uint8_t r = (rgb >> 16) & 0xff;
	uint8_t g = (rgb >> 8)  & 0xff;
	uint8_t b = rgb         & 0xff;
	vecrgb = {r, g, b};
}
