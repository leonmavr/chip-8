#ifndef DISPLAY_HPP
#include <SDL2/SDL.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include "keyboard.hpp" 


class Display : public Keyboard{
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
	bool putPixel(unsigned x, unsigned y);
	void reset();
	void render(); // probably not needed
	void close();
	//unsigned char m_display[32*64];
	void renderAll(unsigned char (&array2D)[32][64]);
	//unsigned char getKey();
	unsigned char m_display[32][64] = {0};


private:
	unsigned m_w;
	unsigned m_h;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	void init();
	void drawPixelXY(unsigned x, unsigned y, unsigned val);
};

#define DISPLAY_HPP 
#endif /* DISPLAY_HPP */
