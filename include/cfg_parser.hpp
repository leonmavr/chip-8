#ifndef CFG_PARSER_HPP
#define CFG_PARSER_HPP

#include "cfg_parser.hpp"
#include "keypad.hpp"
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>

/**
 * Description:
 * Parses a .cfg file for Chip8 emulator roms and reads rom's settings (keypad,
 * frequency.
 */
class CfgParser {
public:
    CfgParser(const std::string& filename);
    int frequency() const { return frequency_; }
    const std::vector<std::pair<std::string, std::string>>& key_descrs() const {
        return key_descrs_;
    }
    bool quirks() const { return quirks_; }

private:
    void ParseConfigFile(const std::string& filename);
    int frequency_;
    const std::unordered_map<uint8_t, std::string> keyboard2keypad_ =
        [input = Keypad::keyboard2keypad]{
        std::unordered_map<uint8_t, std::string> ret;
        for (const auto& pair: input)
            ret[pair.second] = pair.first;
        return ret;
    }();
    std::vector<std::pair<std::string, std::string>> key_descrs_;
    bool quirks_;
    bool cfg_file_found_;
};

#endif // CFG_PARSER_HPP
