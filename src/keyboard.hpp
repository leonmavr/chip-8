#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP 

#include <unordered_map>
#include <SDL2/SDL.h> 


class Keyboard
{
public:
	Keyboard () {
		m_keymap = {
			{SDLK_1, 0x0}, {SDLK_2, 0x1}, {SDLK_3, 0x2}, {SDLK_4, 0x3},
			{SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0x7},
			{SDLK_a, 0x8}, {SDLK_s, 0x9}, {SDLK_d, 0xA}, {SDLK_f, 0xB},
			{SDLK_z, 0xC}, {SDLK_x, 0xD}, {SDLK_c, 0xE}, {SDLK_v, 0xF},
			{SDLK_ESCAPE, 0x10}
		};
	};
	~Keyboard () {};
	unsigned char getKey();

protected:
	std::unordered_map<int,int> m_keymap;
	unsigned char m_kbd[16] = {0};
};
#endif /* KEYBOARD_HPP */
