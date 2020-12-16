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
#include <random>
#include "display.hpp"
#include "keyboard.hpp"

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


class Chip8: public Display, public Keyboard {
public:
	Chip8 () {
		//m_mem.assign(0x1000, 0xff);		// 0xff indicates free space
		memset(m_mem, 0xff, 4096 * sizeof(u8));
		//m_stack[12] = {(u16)0};		// initialise stack
		memset(m_stack, 0x0, 12 * sizeof(u16));
		initFont();
	};
	~Chip8 () {};
	void loadRom(const char* filename, unsigned offset = 0x200);
	void run(unsigned startingOffset = 0x200);

private:
	/* define the architecture */
	//std::vector<u8> m_mem;				// Whole memory
	u8 m_mem[4096];
	u8 m_V[16] = {0};							// V (general) registers
	u8 m_delayTimer, m_soundTimer;
	u16 m_SP = 0;							// Stack pointer
	u16 m_PC = 0x200;						// Program counter
	u16 m_I = 0;							// Index register
	//u8 m_kbd[16] = {0};						// keyboard
	u16 m_opcode;							// current opcode
	bitfields m_bitfields;					// opcode bitfields
	u16 m_stack[12];						// stack
	std::vector<u8> m_fontset;				// font sprites
	void fetch();
	void decode();
	void exec();
	void initFont(unsigned offset = 0x50);
};

#endif /* CHIP8_HPP */
