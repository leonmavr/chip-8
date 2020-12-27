#ifndef LOGGER_HPP
#define LOGGER_HPP 
#include <string>
#include <fstream>

class Logger {
	private:
		Logger() {}
		int iterations = 0;

	public:
		static Logger& getInstance(){
			static Logger instance;
			return instance;
		}
		int log();
		template <typename T, size_t rows, size_t cols>
			std::string array2d2txt(T (&array2D)[rows][cols], std::string outFile = "/tmp/array.txt") {
				std::ofstream ofs;
				ofs.open(outFile, std::ofstream::out | std::ofstream::trunc);
				for (unsigned row = 0; row < rows; row++) {
					for (unsigned col = 0; col < cols; col++) {
						ofs << std::to_string(array2D[row][col]);
					}
					ofs << "\n";
				}
			return outFile;
			}
};

#endif /* LOGGER_HPP */
