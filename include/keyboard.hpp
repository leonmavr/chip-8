#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP 
#include <unordered_map>
#include <SDL2/SDL.h> 
#include <array>
#include <cinttypes>
#include <any>
#include <vector>
#include <iostream> // TODO: remove
#include "ini_reader.hpp" 


class Keyboard: public inireader::IniReader {
public:
	Keyboard (): IniReader() {
		// if ini file contains the following, then map:
		// s_key_x = y
		// y -> s_key_x -> SDLK_z
		std::vector<int> keyboardBuffer;
		for (auto const& s2k: m_str2keypad) {
			std::string key = "s_key_" + s2k.first;
			if (m_iniSettings.count(key) > 0) {
				char clast = s2k.first.back();
				std::string slast(1, clast);
				std::cout << std::stoi(slast, 0, 16) << ", " << std::any_cast<std::string> (m_iniSettings[key]) <<", " << m_str2keypad[slast] << ", " << s2k.first <<std::endl;;
				m_keyQwerty2Chip8[m_str2keypad[slast]] = std::stoi(slast, 0, 16);
				std::cout << m_str2keypad[s2k.first] << " = " << slast << std::endl;
			}
		}
		std::cout << "-----\n";
		m_keyQwerty2Chip8[SDLK_ESCAPE] = 0x10;
		std::unordered_map<unsigned, unsigned> keyQwerty2Chip8 = {
			{SDLK_1, 0x0}, {SDLK_2, 0x1}, {SDLK_3, 0x2}, {SDLK_4, 0x3},
			{SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0x7},
			{SDLK_a, 0x8}, {SDLK_s, 0x9}, {SDLK_d, 0xA}, {SDLK_f, 0xB},
			{SDLK_z, 0xC}, {SDLK_x, 0xD}, {SDLK_c, 0xE}, {SDLK_v, 0xF},
			{SDLK_ESCAPE, 0x10}
		};
		std::cout << "-----\n";
		for (auto const& s2k: keyQwerty2Chip8) {
			std::cout << s2k.first << " = " << s2k.second << std::endl;
		}
	};
	~Keyboard () {};

protected:
	uint8_t getKeyPress();
	std::unordered_map<unsigned, unsigned> m_keyQwerty2Chip8;
	std::array<uint8_t, 16> m_keypresses {};
private:
	std::unordered_map<std::string, unsigned> m_str2keypad = 
	{
		{"a", SDLK_a}, {"b", SDLK_b}, {"c", SDLK_c}, {"d", SDLK_d}, {"e", SDLK_e}, {"f", SDLK_f}, {"g", SDLK_g}, {"h", SDLK_h}, {"i", SDLK_i}, {"j", SDLK_j}, {"k", SDLK_k}, {"l", SDLK_l}, {"m", SDLK_m},
		{"n", SDLK_n}, {"o", SDLK_o}, {"p", SDLK_p}, {"q", SDLK_q}, {"r", SDLK_r}, {"s", SDLK_s}, {"t", SDLK_t}, {"u", SDLK_u}, {"v", SDLK_v}, {"w", SDLK_w}, {"x", SDLK_x}, {"y", SDLK_y}, {"z", SDLK_z},
		{"0", SDLK_0}, {"1", SDLK_1}, {"2", SDLK_2}, {"3", SDLK_3}, {"4", SDLK_4}, {"5", SDLK_5}, {"6", SDLK_6}, {"7", SDLK_7}, {"8", SDLK_8}, {"9", SDLK_9}
	};
};
#endif /* KEYBOARD_HPP */
