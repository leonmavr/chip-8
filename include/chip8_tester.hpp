#ifndef CHIP8_TESTER_HPP
#define CHIP8_TESTER_HPP

#include "chip8.hpp"
#include <iostream>
#include <cstdint>
#include <cassert>


/**
 * This class is responsible for testing the Chip8 class as a whole. The goal 
 * is to verify Chip8's sanity. It does that for example by checking whether
 * the rendered output (frame buffer) is correct.
 */
class Chip8Tester {
    public:
        void AssertFrameBuffer(const Chip8& ch8, const std::array<uint8_t, 64*32>& target)  {
            for (int i = 0; i < target.size(); ++i) {
                //std::cout << (int)ch8.frame_buffer_[i] << ", ";
                assert(ch8.frame_buffer_ == target);
                if (ch8.frame_buffer_[i] != target[i])
                    std::cout << i << std::endl;
            }
        }
        template <typename T>
        void AssertPC(const Chip8& ch8, const T& target) {
            assert(ch8.PC_ == static_cast<uint16_t>(target));
        }
        
    private:

};

#endif // CHIP8_TESTER_HPP
