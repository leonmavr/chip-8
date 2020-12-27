#include "logger.hpp" 
#include <iostream>
#include <fstream>


int Logger::log() {
	return ++iterations;
}


//template <typename T, size_t rows, size_t cols>
//void array2d2txt(T (&array2D)[rows][cols]) {
