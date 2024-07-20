#ifndef CFG_PARSER_HPP
#define CFG_PARSER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

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
    const std::map<char, uint8_t>& GetKeyMap() const { return key_map_; }

private:
    void ParseConfigFile(const std::string& filename);
    int frequency_;
    std::map<char, uint8_t> key_map_;
};

#endif // CFG_PARSER_HPP
