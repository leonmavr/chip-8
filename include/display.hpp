#ifndef DISPLAY_HPP
#define DISPLAY_HPP 
#include <SDL2/SDL.h>
#include <unordered_map>
#include <cinttypes>
#include "keyboard.hpp" 


class Display {
public:
	Display (unsigned w = 640, unsigned h = 320) {
		m_w = w;
		m_h = h;
		init();	
	};
	~Display () { 
		free(m_window);
		free(m_renderer);
	};

protected:
	void cls();									// clear display window 
	void close();								// close display window 
	void renderAll(uint8_t (&array2D)[32][64]); // render the input 2D array in the display window; values of 1 rendered to white, 0 to black
	uint8_t m_display[32][64] = {0};

private:
	unsigned m_w;
	unsigned m_h;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;
	/**
	 * @brief Initialises m_window and m_renderer, also clears the display window
	 */
	void init();
	/**
	 * @brief Draws pixel at scaled location (x, y) (scaled by size of display window). If colour == 0 draws black, else white
	 *
	 * @param x	x-ordinate of pixel to be drawn
	 * @param y	y-ordinate of pixel to be drawn
	 * @param colour Pixel colour; 0 for black, 1 for white
	 */
	void drawPixelXY(unsigned x, unsigned y, unsigned colour);
};

#endif /* DISPLAY_HPP */
