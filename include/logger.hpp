#ifndef LOGGER_HPP
#define LOGGER_HPP 
#include <string>
#include <fstream>

/**
 * @brief	Singleton class to log data from Chip8's execution, particularly the current iteration number and the screen data.
 * 			Usage example:
 *			Logger::getInstance().screendump(m_display);
 *			Logger::getInstance().count()
 *
 */
class Logger {
	private:
		Logger() {}
		int iterations = 0;

	public:
		static Logger& getInstance(){
			static Logger instance;
			return instance;
		}
		int count(); // return and then increment the `iterations` member
		/**
		 * @brief Dump data from a 2D input array to a text file. Nonzero values are dumped as 1s and zeros as 0s
		 *
		 * @param &array2D 2D array of any type, as long as it can be converted to string (std::to_string)
		 * @param outFile The file where to dump data
		 *
		 * @return the outFile argument
		 */
		template <typename T, size_t rows, size_t cols>
			std::string screendump(T (&array2D)[rows][cols], std::string outFile = "/tmp/array.txt") {
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
