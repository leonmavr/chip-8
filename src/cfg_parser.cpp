#include "cfg_parser.hpp"
#include <cstdint>
#include <algorithm>
#include <iostream>

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

CfgParser::CfgParser(const std::string& filename) : frequency_(0) {
    ParseConfigFile(filename);
}

void CfgParser::ParseConfigFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace from the beginning of the line
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
            [](unsigned char ch) { return !std::isspace(ch); }));

        if (line.empty() || line[0] == '#' || IsWhitespaceOnly(line))
            continue;

        std::istringstream iss(line);
        std::string key;
        std::string value;
        if (std::getline(iss, key, ',') && std::getline(iss, value)) {
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
            value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

            if (key.length() == 1 && value.length() > 2 && value.substr(0, 2) == "0x") {
                char keyChar = key[0];
                uint8_t hexValue = static_cast<uint8_t>(std::stoi(value, nullptr, 16));
                key_map_[keyChar] = hexValue;
            } 
        } else {
            try {
                frequency_ = std::stoi(line); // Attempt to convert the string to an integer
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            }
        }
    }
    file.close();
}
