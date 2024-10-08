#include "cfg_parser.hpp"
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <unordered_map>

/*
 * Description:
 * Parses a .cfg file for Chip8 emulator roms and fetches rom's settings (keypad,
 * frequency.
 */

static bool IsWhitespaceOnly(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
}

static void Trim(std::string &key) {
    // Remove spaces from the beginning and from the end
    key.erase(key.begin(), std::find_if_not(key.begin(), key.end(), ::isspace));
    key.erase(std::find_if_not(key.rbegin(), key.rend(), ::isspace).base(), key.end());
}


CfgParser::CfgParser(const std::string& filename) :
    frequency_(350),
    quirks_(true),
    cfg_file_found_(true) {
    ParseConfigFile(filename);
}

void CfgParser::ParseConfigFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cfg_file_found_ = false;
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
            Trim(key);
            Trim(value);
            uint8_t hex_key = static_cast<uint8_t>(std::stoi(key.substr(2), nullptr, 16));
            key_descrs_.push_back(std::make_pair(std::string(keyboard2keypad_.at(hex_key)), value));
        } else {
            std::string line_lower = line;
            std::transform(line_lower.begin(), line_lower.end(), line_lower.begin(),
                   [](char c){ return std::tolower(c); });
            bool contains_quirks = line_lower == "on" || line_lower == "off";
            bool contains_freq = !line_lower.empty() &&
                std::all_of(line_lower.begin(), line_lower.end(), ::isdigit);
            quirks_ = (contains_quirks && line_lower == "on") ? true :
                (contains_quirks && line_lower == "off") ? false : quirks_;
            if (contains_freq)
                frequency_ = std::stoi(line);
        }
    }
    file.close();
}
