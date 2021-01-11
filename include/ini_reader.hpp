#ifndef INI_READER_HPP
#define INI_READER_HPP 

#include <string>
#include <unordered_map>
#include <any>
#include "ini_reader.hpp" 

namespace inireader {
class IniReader {
	public:
		IniReader () { readIni(); };
		IniReader (std::string filename) { readIni(filename); };
		~IniReader () {};
		std::unordered_map<std::string, std::any> m_iniSettings {};
	private:
		void readIni(std::string filename = "chip8.ini");
};

}
#endif /* INI_READER_HPP */
