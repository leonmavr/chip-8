#include <fstream>
#include <algorithm> // remove()
#include "ini_reader.hpp"

void IniReader::readIni(std::string filename) {
	std::ifstream filestream(filename);
	std::string line;
	std::string delim = "=";
	auto onlyContainsSpaces = [] (std::string str) -> bool
		{ return str.find_first_not_of(" \t\n\v\f\r") == std::string::npos; };
	auto stringRemoveSpaces = [] (std::string& str)
		{ str.erase(std::remove(str.begin(), str.end(), ' ')); };

	if (!filestream.good())
		throw std::runtime_error("Cannot find .ini file.");

	while (std::getline(filestream, line)) {
		if (line.find(delim) != std::string::npos) {
			std::string key = line.substr(0, line.find(delim));
			stringRemoveSpaces(key);
			std::string val = line.substr(line.find(delim)+1, line.find(delim));
			if (!val.empty() && !onlyContainsSpaces(val)) // integer good to insert
				m_iniSettings[key] = std::stoi(val);
		}
	}	
}
