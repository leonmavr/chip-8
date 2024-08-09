#ifndef CHIP8_TESTER_HPP
#define CHIP8_TESTER_HPP

#include "chip8.hpp"
#include <iostream>


class Chip8Tester {
    public:
        void foo(const Chip8& ch8)  {std::cout << ch8.PC_ << std::endl;}
    private:

};

#endif // CHIP8_TESTER_HPP
