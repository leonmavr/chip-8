#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include <iostream>
#include <fstream>
#include <memory>
#include <array>
#include <algorithm> // fill()
#include <cinttypes> // uint8_t, uint16_t
#include <bits/stdc++.h>
#include "display.hpp"
#include "keyboard.hpp"
#include "bitfields.hpp"


enum {
	SPEED_NORMAL,								// Opcodes per sec configured by user 
	SPEED_FAST,									// Approaching overclock
	SPEED_OVERCLOCK								// As fast as host computer allows
};


class Chip8: public Display, public Keyboard {
public:
	Chip8 () {
		init();
	};
	Chip8 (unsigned cpuSpeed, unsigned instrPerSec) {
		init(cpuSpeed, instrPerSec);
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
	unsigned m_clockSpeed;						// CPU speed
	bitfields m_bitfields;						// opcode bitfields
	std::array<uint16_t, 12>m_stack;			// stack
	std::vector<uint8_t> m_fontset;				// font sprites
	uint8_t rand();								// Chip8 has a random number generator 
	unsigned m_instrPerSec;						// instr. per sec; helps with timing

	inline void fetch();						// handles current instruction
	inline void decode();						// handles current instrution
	void exec();								// handles current instrution
	void init(unsigned clockSpeed = SPEED_NORMAL, unsigned instrPerSec = 400);
};

#endif /* CHIP8_HPP */