#ifndef DISPLAY_HPP
#include <SDL2/SDL.h>


class Display {
public:
	Display (unsigned w = 640, unsigned h = 320) {
		m_w = w;
		m_h = h;
		init();	
	};
	~Display () { };
	bool putPixel(unsigned x, unsigned y);
	void reset();
	void render();
	void close();
	unsigned char m_display[32*64];

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
