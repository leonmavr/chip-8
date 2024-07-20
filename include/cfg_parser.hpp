#ifndef CFG_PARSER_HPP
#define CFG_PARSER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

class ConfigParser {
public:
    ConfigParser(const std::string& filename);

    int getFrequency() const { return frequency_; }
    const std::map<char, uint8_t>& getKeyMap() const { return key_map_; }

private:
    void parseConfigFile(const std::string& filename);

    int frequency_;
    std::map<char, uint8_t> key_map_;
};


#endif // CFG_PARSER_HPP
