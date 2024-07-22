#ifndef CFG_PARSER_HPP
#define CFG_PARSER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <unordered_map>

/*
 * Description:
 * Parses a .cfg file for Chip8 emulator roms and fetches rom's settings (keypad,
 * frequency.
 *
 * Creation date:
 * 20/07/2024 
 *
 * Notes:
 * Originally written by ChatGPT.
 * Tweaked by project's author.
 */

class CfgParser {
public:
    CfgParser(const std::string& filename);
    int GetFrequency() const { return frequency_; }
    const std::unordered_map<std::string, std::string>& GetKeyMap() const { return key_map_; }

private:
    void ParseConfigFile(const std::string& filename);
    int frequency_;
    std::unordered_map<std::string, std::string> key_map_;
    // TODO: find chip8 keyboard2keypad_ and reverse it
    std::unordered_map<uint8_t, std::string> keyboard2keypad_ = {
            {0x1, "1"}, {0x2, "2"}, {0x3, "3"}, {0xC, "4"}, {0x4, "q"}, {0x5, "w"},
            {0x6, "e"}, {0xD, "r"}, {0x7, "a"}, {0x8, "s"}, {0x9, "d"}, {0xE, "f"},
            {0xA, "z"}, {0x0, "x"}, {0xB, "c"}, {0xF, "v"}
        };
};

#endif // CFG_PARSER_HPP
