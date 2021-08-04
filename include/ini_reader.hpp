#ifndef INI_READER_HPP
#define INI_READER_HPP 

#include <string>
#include <unordered_map>
#include <any>

class IniReader {
    public:
        IniReader ()  { readIni(); };
        IniReader (std::string filename) { readIni(filename); };
        ~IniReader () {};
        std::unordered_map<std::string, std::any> m_iniSettings {};
    private:
        void readIni(const std::string& filename = "chip8.ini");
};

#endif /* INI_READER_HPP */
