#include "cfg_parser.hpp"
#include <cstdint>
#include <algorithm>
#include <iostream>
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

static bool IsWhitespaceOnly(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
}

CfgParser::CfgParser(const std::string& filename) : frequency_(250), cfg_file_found_(false) {
    ParseConfigFile(filename);
}

void CfgParser::ParseConfigFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cfg_file_found_ = false;
        key_map_[""] = ""; 
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || IsWhitespaceOnly(line))
            continue;
        // Trim whitespace from the beginning of the line
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));

        std::istringstream iss(line);
        std::string key;
        std::string value;
        std::unordered_map<std::string, std::string> keymap;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
            value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
            uint8_t hex_key = static_cast<uint8_t>(std::stoi(key.substr(2), nullptr, 16));
            key_map_[keyboard2keypad_[hex_key]] = value;
        } else {
            try {
                frequency_ = std::stoi(line);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            }
        }
    }
    file.close();
}
