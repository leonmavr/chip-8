#ifndef DISPLAY_HPP
#include <SDL2/SDL.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>


class Display {
public:
	Display (unsigned w = 640, unsigned h = 320) {
		m_w = w;
		m_h = h;
		m_keymap = {
			{SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
			{SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
			{SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
			{SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},
			{SDLK_5, 0x5}, {SDLK_6, 0x6}, {SDLK_7, 0x7},
			{SDLK_8, 0x8}, {SDLK_9, 0x9}, {SDLK_0, 0x0}, {SDLK_ESCAPE,-1}
		};
		std::cout << "keymap init[0]: " << m_keymap[0] << std::endl;
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
	unsigned char getKey();
	unsigned char m_display[32][64] = {0};
	unsigned char m_kbd[16] = {0};
	std::deque<unsigned char> m_keys;


private:
	unsigned m_w;
	unsigned m_h;
	std::unordered_map<int,int> m_keymap;
	SDL_Window* m_window;
	SDL_Renderer* m_renderer;

	void init();
	void drawPixelXY(unsigned x, unsigned y, unsigned val);
};

#define DISPLAY_HPP 
#endif /* DISPLAY_HPP */
