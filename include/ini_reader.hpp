#ifndef INI_READER_HPP
#define INI_READER_HPP 

#include <string>
#include <unordered_map>

class IniReader {
	protected:
		IniReader () { readIni(); };
		IniReader (std::string filename) { readIni(filename); };
		~IniReader () {};
		std::unordered_map<std::string, int> m_iniSettings {};
	private:
		void readIni(std::string filename = "../chip8.ini");
};

#endif /* INI_READER_HPP */
