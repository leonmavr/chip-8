#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include <iostream>
#include <fstream>
#include <memory>
#include <array>
#include <algorithm> // fill()
#include <cinttypes>
#include <bits/stdc++.h>
#include "display.hpp"
#include "keyboard.hpp"


typedef struct bitfields_t {
	/* see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0 */
	unsigned type;
	unsigned n;
	unsigned y;
	unsigned x;
	unsigned kk;
	unsigned nnn;
} bitfields;


class Chip8: public Display, public Keyboard {
public:
	Chip8 () {
		init();
	};
	~Chip8 () {};
	void loadRom(const char* filename, unsigned offset = 0x200);
	void run(unsigned startingOffset = 0x200);

private:
	/* define the architecture */
	std::array<uint8_t, 0x1000> m_mem {};		// Whole memory
	std::array<uint8_t, 16> m_V {};				// V (general) registers
	uint8_t m_delayTimer, m_soundTimer;			// timers
	uint16_t m_SP;								// Stack pointer
	uint16_t m_PC;								// Program counter
	uint16_t m_I;								// Index register
	uint16_t m_opcode;							// current opcode
	bitfields m_bitfields;						// opcode bitfields
	std::array<uint16_t, 12>m_stack;			// stack
	std::vector<uint8_t> m_fontset;				// font sprites
	void fetch();
	void decode();
	void exec();
	void init();
};

#endif /* CHIP8_HPP */
