#include "logger.hpp" 
#include <iostream>
#include <fstream>


int Logger::log() {
	// TODO: it's better to implement screendump (2d array -> file) here
	return ++iterations;
}


//template <typename T, size_t rows, size_t cols>
//void array2d2txt(T (&array2D)[rows][cols]) {
