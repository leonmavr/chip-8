#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP 
#include <unordered_map>
#include <SDL2/SDL.h> 
#include <array>
#include <cinttypes>
#include <any>
#include <vector>
#include <algorithm> // find_if
#include <iostream> // TODO: remove
#include "ini_reader.hpp" 


// copies keys-value pairs to new map only if key starts froom (first 2 charts) a certain string
static void copyOnlyForPrefix(
		std::unordered_map<std::string, std::any> map,
		std::unordered_map<std::string, std::any>& mapNew,
		std::string prefix)
{
	for (const auto& keyval: map) {
		if (keyval.first.substr(0, prefix.length()) == prefix)
			mapNew.insert({keyval.first, keyval.second});
	}
}



class Keyboard: public inireader::IniReader {
public:
	Keyboard (): IniReader() {
		// if ini file contains the following, then map:
		// s_key_x = y
		// y -> s_key_x -> SDLK_z
		std::vector<int> keyboardBuffer;
		// copy only strings to new map
		decltype(m_iniSettings) m_iniSettingsCpy;
		copyOnlyForPrefix(m_iniSettings, m_iniSettingsCpy, "s_key");

		std::string chip8Key;
		bool success = false;
		for (auto const& s2k: m_str2keypad) {
			// this is the `y`
			std::string key = s2k.first;
			// check if m_iniSettings *values* contain key (y from above) 
			for (const auto& iniPair: m_iniSettingsCpy){
				success = false;
				// fetch (from .ini file) only key-value pairs whose key starts with "s_key" - these are the Chip8 key-values
					if (std::any_cast<std::string> (iniPair.second).compare(key) == 0) {
						success = true;
						std::string chip8Key (1, iniPair.first.back());
						chip8Key = iniPair.first.substr(iniPair.first.size() - 1);
						m_ch8key = chip8Key;
						break;
					}
			}
			//std::cout <<typeid (result ).name();
			if (success == true) {
				// "s" -> SDLK_s -> 0xb
				std::string slast(1, s2k.first.back());
				m_keyQwerty2Chip8[m_str2keypad[slast]] = std::stoi(m_ch8key, 0, 16);
				std::cout << m_str2keypad[slast] << " -> "<<  m_keyQwerty2Chip8[m_str2keypad[slast]]<< " -> " << std::stoi(m_ch8key, 0, 16) << std::endl;
			}
		}
		m_keyQwerty2Chip8[SDLK_ESCAPE] = 0x10;
	};
	~Keyboard () {};

protected:
	uint8_t getKeyPress();
	std::string m_ch8key;
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
