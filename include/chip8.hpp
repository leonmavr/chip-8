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


class Chip8: public Display, public Keyboard {
public:
	// don't forget to initialise constant members 
	Chip8 (std::string fnameIni):
		Keyboard(fnameIni),
		m_instrPerSec(std::any_cast<int>(m_iniSettings["i_instructions_per_sec"])),
		m_mute(std::any_cast<bool>(m_iniSettings["b_mute"])),
		m_overclock(std::any_cast<bool>(m_iniSettings["b_overclock"])),
		m_maxIter(std::any_cast<int>(m_iniSettings["i_stop_after_iter"])),
		m_freq(std::any_cast<int>(m_iniSettings["i_freq"]))
	{
			init();
	};
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
	uint16_t m_opcode;							// current opcode
	unsigned m_clockSpeed;						// clock speed (enum); normal or overclocked
	bitfields m_bitfields;						// opcode bitfields
	std::array<uint16_t, 12>m_stack;			// stack
	std::vector<uint8_t> m_fontset;				// font sprites
	uint8_t rand();								// Chip8 has a random number generator 

	inline void fetch();						// handles current instruction
	inline void decode();						// handles current instruction
	void exec();								// handles current instruction

	void init();								// initialises memory, registers, and configs
	
	const bool m_overclock;						// config flag; if true, host runs the rom as fast as possible and on mute
	const int m_instrPerSec;					// config flag; defines the CPU speed; how many instructions to run per sec
	const int m_maxIter;						// config flag; how many CPU cycles to run before terminating. If 0, run forever. Used for unit testing.
	const bool m_mute;							// config flag; if true; run on mute
	const float m_freq;							// config flag; frequency of played sound
};

#endif /* CHIP8_HPP */
