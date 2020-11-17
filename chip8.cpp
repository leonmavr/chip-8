#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <bits/stdc++.h>

typedef unsigned char byte;
typedef unsigned short word;

typedef struct bitfields_t {
	/* see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0 */
	unsigned type;
	unsigned n;
	unsigned y;
	unsigned x;
	unsigned kk;
	unsigned nnn;
} bitfields;


class Chip8 {
public:
	Chip8 () {
		m_mem.assign(0x1000, 0xff);		// 0xff indicates free space
		initFont();
	};
	virtual ~Chip8 () {};
	void loadRom(const char* filename, unsigned offset);
	void fetch();
	void decode();
	void exec();
	void run(unsigned startingOffset);

private:
	/* define the architecture */
	std::vector<int> m_mem;					// Whole memory
	byte m_V[16];							// V (general) registers
	byte m_display[64*32];
	byte m_delayTimer, m_soundTimer;
	word m_SP = 0;							// Stack pointer
	word m_PC = 0x200;						// Program counter
	word m_I = 0;							// Index register
	byte m_kbd[16];							// keyboard
	word m_opcode;							// current opcode
	bitfields m_bitfields;					// opcode bitfields
	word m_stack[12];						// stack
		typedef struct opcodeEntry_t {
		char opcode[5];
		void (*func) (void);
	} opcodeEntry;
	void initFont(unsigned offset = 0x50);
};


void Chip8::loadRom(const char* filename, unsigned offset = 0x200) {
	// adapted from https://bisqwit.iki.fi/jutut/kuvat/programming_examples/chip8/chip8.cc
	for(std::ifstream f(filename, std::ios::binary); f.good(); )
		m_mem.at(offset++ & 0xFFF) = f.get();
}


void Chip8::initFont(unsigned int offset) {
	// define font sprites - see https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
	std::vector<byte> fontset = 
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	// write to m_mem
	for (byte element: fontset)
		m_mem.at(offset++ & 0xFF) = element;
}


void Chip8::fetch() {
	// copy the current byte of the program to the current instruction
	// TODO: if host little endian else m_mem[m_PC] << 8 | m_mem[m_PC+1]
	m_opcode = m_mem[m_PC+1] << 8 | m_mem[m_PC];	
}


void Chip8::decode() {
	/*
	 * Extract bit-fields from the opcode
	 * see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
	 * for bitfield explanation
	 */
	m_bitfields.type = (m_opcode >> 12) & 0xf;
	m_bitfields.n    = m_opcode         & 0xf;
	m_bitfields.y    = (m_opcode >> 8)  & 0xf;
	m_bitfields.x    = (m_opcode >> 4)  & 0xf;
	m_bitfields.kk   = m_opcode         & 0xff;
	m_bitfields.nnn  = m_opcode         & 0xfff;
	//std::cout << "decode " << std::hex << m_opcode << std::endl;
	std::cout << "n = " << m_bitfields.n << ", x = " << m_bitfields.x << ", y = " << m_bitfields.y << ", kk = " << m_bitfields.kk << ", nnn = " << m_bitfields.nnn << ", type = " << m_bitfields.type << std::endl;
}


void Chip8::exec() {
	// TODO: switch statement for bitfields to find right instruction
	//
	//std::cout << "exec: " << std::hex  << m_bitfields.type << std::endl;
	switch(m_bitfields.type) {
		case 0x0:
			;
		case 0x1:
			;
		case 0x2:
			;
		case 0x3:
			;
		case 0x4:
			;
		case 0x5:
			;
		case 0x6:
			;
		case 0x7:
			;
		case 0x8:
			;
		case 0x9:
			;
		case 0xa:
			;
		case 0xb:
			;
		case 0xc:
			;
		case 0xd:
			;
		case 0xe:
			;
		case 0xf:
			;
	}
	// move to the next instruction
	m_PC += 2;
}


void Chip8::run(unsigned startingOffset = 0x200) {
	m_PC = startingOffset;
	std::cout << "----------------------------" << std::endl;

	while ((m_mem.at(m_PC) != 0xff) || (m_mem.at(m_PC+1) != 0xff)) {
		std::cout << std::hex <<m_mem.at(m_PC) << std::endl;;
		std::cout << std::hex << m_mem.at(m_PC+1) << std::endl;;
		Chip8::fetch();
		Chip8::decode();
		Chip8::exec();
	}
	
}



int main(int argc, char *argv[])
{
	if (argc != 2) {
		//std::cout << "Usage: ./<this_program> <rom_file>";

		//return 1;
	}

	auto ch8 = std::make_unique<Chip8>();
	ch8->loadRom("tetris.c8");
	ch8->run();
	//ch8->initFont();
	//ch8->fetch();
	//ch8->decode();
	//ch8->exec();

	return 0;
}
