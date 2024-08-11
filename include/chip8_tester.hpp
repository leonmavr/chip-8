#ifndef CHIP8_TESTER_HPP
#define CHIP8_TESTER_HPP

#include "chip8.hpp"
#include <iostream>
#include <cassert>


/**
 * This class is responsible for testing the Chip8 class as a whole. The goal 
 * is to verify Chip8's sanity. It does so by checking the internal state of
 * Chip8, e.g. the rendered output (frame buffer), PC, etc.
 */
class Chip8Tester {
    public:
        template <typename T>
        void AssertFrameBuffer(const Chip8& ch8, const std::array<T, 64*32>& target)  {
            for (int i = 0; i < target.size(); ++i) {
                assert(static_cast<int>(ch8.frame_buffer_[i]) ==
                static_cast<int>(target[i]));
            }
        }

        template <typename T>
        void AssertPC(const Chip8& ch8, const T& target) {
            assert(static_cast<int>(ch8.PC_) == static_cast<int>(target));
        }
        
    private:

};

#endif // CHIP8_TESTER_HPP
