#include <fstream>
#include <algorithm> // remove()
#include <string>
#include <cstring> // strcasecmp
#include "ini_reader.hpp" 


void IniReader::readIni(std::string filename) {
	std::ifstream filestream(filename);
	std::string line;
	std::string delim = "=";
	if (!filestream.good())
		throw std::runtime_error("Cannot find .ini file.");

	// auxiliary string checking functions
	auto onlyContainsSpaces = [] (const std::string& str) -> bool
		{ return str.find_first_not_of(" \t\n\v\f\r") == std::string::npos; };
	auto stringRemoveSpaces = [] (std::string& str)
		{ str.erase(std::remove(str.begin(), str.end(), ' ')); };
	// credits Andre Holzner: https://stackoverflow.com/a/3613327
	auto strIsTrue = [] (const std::string& str) -> bool
		{ return strcasecmp("true", str.c_str()) == 0; };
	auto lineIsComment = [] (const std::string& str) -> bool
		{ return str.rfind("#", 0) == 0; };

	while (std::getline(filestream, line)) {
		if ((line.find(delim) != std::string::npos) && (!lineIsComment(line))) {
			std::string key = line.substr(0, line.find(delim));
			stringRemoveSpaces(key);
			std::string val = line.substr(line.find(delim)+1, line.find(delim));
			if (!val.empty() && !onlyContainsSpaces(val)) { // line good to insert 
				std::string prefix = key.substr(0, 2);
				stringRemoveSpaces(val);
				if (prefix == "i_") // integer
					m_iniSettings[key] = std::stoi(val);
				else if (prefix == "b_") // boolean ("true/false")
					m_iniSettings[key] = strIsTrue(val);
				else if (prefix == "s_") // string
					m_iniSettings[key] = val;
			}
		}
	}	
}
