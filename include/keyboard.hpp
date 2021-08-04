#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP 
#include <unordered_map>
#include <SDL2/SDL.h> 
#include <array>
#include <cinttypes>
#include <any>
#include <algorithm> // find_if
#include "ini_reader.hpp" 


/**
 * @brief Copies keys-value pairs to new a map only if key starts from a certain string
 *
 * @param map An input map to select entries from
 * @param mapNew An output map where filtered entries from input will be stored to
 * @param prefix If they key of each entry of input starts from `prefix`, select it and store it to mapNew
 */
static void copyOnlyForPrefix(
        const std::unordered_map<std::string, std::any>& map,
        std::unordered_map<std::string, std::any>& mapNew,
        std::string prefix)
{
    for (const auto& keyval: map) {
        if (keyval.first.substr(0, prefix.length()) == prefix)
            mapNew.insert({keyval.first, keyval.second});
    }
}


class Keyboard: virtual public IniReader {
    public:
        Keyboard (std::string iniFile): IniReader(iniFile) {
            // from ini file, select entries that start with s_key, e.g. s_key_5 = w
            decltype(m_iniSettings) iniSettingsCpy;
            copyOnlyForPrefix(m_iniSettings, iniSettingsCpy, "s_key");

            for (const auto& key2sdl: m_keyboard2sdl) {
                std::string key = key2sdl.first; // keyboard key
                // if keyboard key (`key`) is in the rhs of the .ini file (value of `iniSettingsCpy`)
                auto result = std::find_if(iniSettingsCpy.begin(), iniSettingsCpy.end(),
                        [key](const auto& map) {return std::any_cast<std::string>(map.second) == key; });

                if (result != iniSettingsCpy.end()) {
                    unsigned ch8key = std::stoi(result->first.substr(result->first.size() - 1), 0, 16); // result = <s_key_X, Y>
                    std::string slast(1, key2sdl.first.back()); // s_key_X => X, X is in hex 
                    m_keyQwerty2Chip8[m_keyboard2sdl[slast]] = ch8key; // X -> [SDL key for X]
                }
            }
            // Add the Esc key for the UI
            m_keyQwerty2Chip8[SDLK_ESCAPE] = 0x10;
        };

        ~Keyboard () {};

    protected:
        /**
         * @brief Waits for keypress and then returns a keypad (0x0 - 0xf). This function is required by chip8's architecure
         *
         * @return The keypad key corresponding to pressed keyboard key (0x0-0xf)
         */
        uint8_t getKeyPress();
        std::unordered_map<unsigned, unsigned> m_keyQwerty2Chip8;
        std::array<uint8_t, 16> m_keypresses {};

    private:
        std::unordered_map<std::string, unsigned> m_keyboard2sdl = 
        {
            {"a", SDLK_a}, {"b", SDLK_b}, {"c", SDLK_c}, {"d", SDLK_d}, {"e", SDLK_e}, {"f", SDLK_f},
            {"g", SDLK_g}, {"h", SDLK_h}, {"i", SDLK_i}, {"j", SDLK_j}, {"k", SDLK_k}, {"l", SDLK_l},
            {"m", SDLK_m}, {"n", SDLK_n}, {"o", SDLK_o}, {"p", SDLK_p}, {"q", SDLK_q}, {"r", SDLK_r},
            {"s", SDLK_s}, {"t", SDLK_t}, {"u", SDLK_u}, {"v", SDLK_v}, {"w", SDLK_w}, {"x", SDLK_x},
            {"y", SDLK_y}, {"z", SDLK_z},
            {"0", SDLK_0}, {"1", SDLK_1}, {"2", SDLK_2}, {"3", SDLK_3}, {"4", SDLK_4}, {"5", SDLK_5},
            {"6", SDLK_6}, {"7", SDLK_7}, {"8", SDLK_8}, {"9", SDLK_9}
        };
};
#endif /* KEYBOARD_HPP */
