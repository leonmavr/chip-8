#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <bits/stdc++.h>
#include <random>
#include <chrono>
#include <thread>
#include "display.hpp"

typedef unsigned char u8;
typedef unsigned short u16;

typedef struct bitfields_t {
	/* see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0 */
	unsigned type;
	unsigned n;
	unsigned y;
	unsigned x;
	unsigned kk;
	unsigned nnn;
} bitfields;


class Chip8: public Display {
public:
	Chip8 () {
		m_mem.assign(0x1000, 0xff);		// 0xff indicates free space
		m_stack.assign(12, 0x0);		// initialise stack
		initFont();
	};
	~Chip8 () {};
	void loadRom(const char* filename, unsigned offset = 0x200);
	void run(unsigned startingOffset = 0x200);

private:
	/* define the architecture */
	std::vector<u16> m_mem;				// Whole memory
	u8 m_V[16];							// V (general) registers
	u8 m_display[64*32];
	u8 m_delayTimer, m_soundTimer;
	u16 m_SP = 0;							// Stack pointer
	u16 m_PC = 0x200;						// Program counter
	u16 m_I = 0;							// Index register
	u8 m_kbd[16];							// keyboard
	u16 m_opcode;							// current opcode
	bitfields m_bitfields;					// opcode bitfields
	std::vector<u16> m_stack;				// stack
	void fetch();
	void decode();
	void exec();
	void initFont(unsigned offset = 0x50);
};

#endif /* CHIP8_HPP */
