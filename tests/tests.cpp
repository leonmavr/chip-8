#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "chip8.hpp"
#include "display.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>


// credits mtrw: https://stackoverflow.com/a/37575457
bool compareFiles(const std::string& p1, const std::string& p2) {
	std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
	std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);
	if (f1.fail() || f2.fail()) {
		return false; //file problem
	}

	if (f1.tellg() != f2.tellg()) {
		return false; //size mismatch
	}

	//seek back to beginning and use std::equal to compare contents
	f1.seekg(0, std::ifstream::beg);
	f2.seekg(0, std::ifstream::beg);
	return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
			std::istreambuf_iterator<char>(),
			std::istreambuf_iterator<char>(f2.rdbuf()));
}


TEST_CASE("Check screen output for IBM logo rom") {
	auto ch8 = std::make_unique<Chip8>("tests/chip8.ini");
	ch8->loadRom("roms/IBM_logo.ch8");
	SECTION("Compare screen output", "[screen]") {
		// when compiled with MAX_ITER defined, output at /tmp/array.txt
		ch8->run();	 
		// compare the test's output with the correct one (outputs folder)
		bool res = compareFiles("tests/outputs/ibm_logo.txt", "/tmp/array.txt");
		REQUIRE(res == true);
	}
}
