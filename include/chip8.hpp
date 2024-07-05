#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include <iostream>
#include <fstream>
#include <memory>
#include <array>
#include <cstdint>
#include <algorithm> // fill()
#include <cinttypes> // uint8_t, uint16_t
#include <bits/stdc++.h>
#include "display.hpp"
#include "keyboard.hpp"
#include "ini_reader.hpp"
#include "bitfields.hpp"


typedef struct opcode_t {
    uint8_t prefix : 4; 
    uint8_t x : 4; 
    uint8_t y : 4; 
    uint8_t n : 4; 
    uint8_t nn : 8; 
    uint16_t nnn : 12; 
} opcode_t;


class Chip8: public Display, public Keyboard, virtual public IniReader {
    public:
        // don't forget to initialise constant members 
        Chip8 (std::string fnameIni);
        ~Chip8 () {};
        /**
         * @brief 				Load a rom from a filepath
         *
         * @param filename		File name of rom
         * @param offset		Starting offset in Chip8's memory. Rom data will be written to this offset onwards (towards higher addresses).
         * 						`offset` is part of the architecture to it should be left to the default value of 0x200	.
         */
        void loadRom(const char* filename, unsigned offset = 0x200);
        /**
         * @brief 
         *
         * @param startingOffset Run the rom data that are starting an offset. Again, the standard offset for the rom is 0x200.
         */
        void run(unsigned startingOffset = 0x200);

    private:
        /* define the architecture */
        std::array<uint8_t, 0x1000> m_mem {};		// Whole memory
        std::array<uint8_t, 16> m_V {};				// V (general) registers
        uint8_t m_delayTimer, m_soundTimer;			// timers
        uint16_t m_SP;								// Stack pointer
        uint16_t m_PC;								// Program counter
        uint16_t m_I;								// Index register
        unsigned m_clockSpeed;						// clock speed (enum); normal or overclocked
        std::array<uint16_t, 12>m_stack;			// stack
        std::vector<uint8_t> m_fontset;				// font sprites
        uint8_t rand();								// Chip8 has a random number generator 

        uint16_t fetch();                           // handles current instruction
        opcode_t decode(uint16_t instr);            // handles current instruction
        void exec(opcode_t opc);                    // handles current instruction

        void init();								// initialises memory, registers, and configs

        const int m_instrPerSec;					// config flag; defines the CPU speed; how many instructions to run per sec
        const int m_maxIter;						// config flag; how many CPU cycles to run before terminating. If 0, run forever. Used for unit testing.
        const bool m_mute;							// config flag; if true; run on mute
        const float m_freq;							// config flag; frequency of played sound
};

#endif /* CHIP8_HPP */
