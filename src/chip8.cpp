#include "chip8.hpp" 
#include <iostream>
#include <random>
#include <cstdlib>

void Chip8::loadRom(const char* filename, unsigned offset) {
	// adapted from https://bisqwit.iki.fi/jutut/kuvat/programming_examples/chip8/chip8.cc
	for(std::ifstream f(filename, std::ios::binary); f.good(); ) 
		m_mem[offset++ & 0xFFF] = f.get();
}


void Chip8::initFont(unsigned int offset) {
	// define font sprites - see https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
	std::vector<u8> m_fontset = 
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
	for (u8 element: m_fontset)
		m_mem[offset++ & 0xFF] = element;
}


void Chip8::fetch() {
	assert(m_PC < 4096);
	// copy the current byte of the program to the current instruction,
	// assuming little endian host
	m_opcode = m_mem[m_PC] << 8 | m_mem[m_PC+1];
}


void Chip8::decode() {
	/*
	 * Extract bit-fields from the opcode
	 * see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
	 * for bitfield explanation
	 */
	m_bitfields.type = (m_opcode >> 12) & 0x000f;
	m_bitfields.n    = m_opcode         & 0x000f;
	m_bitfields.x    = (m_opcode >> 8)  & 0x000f;
	m_bitfields.y    = (m_opcode >> 4)  & 0x000f;
	m_bitfields.kk   = m_opcode         & 0x00ff;
	m_bitfields.nnn  = m_opcode         & 0x0fff;
}

static void drawSprite(u8 sprite, u8 height) {
	for (int xx = 0; xx < 8; xx++) {
		if ((sprite && 0x80) > 0)
			std::cout << "o";
		else
			std::cout << ".";
		sprite <<= 1;
		}
	std::cout << std::endl;
}

void Chip8::exec() {
	const auto x = m_bitfields.x;
	const auto y = m_bitfields.y;
	const auto kk = m_bitfields.kk;
	const auto nnn = m_bitfields.nnn;
	const auto n = m_bitfields.n;
	switch(m_bitfields.type) {
		case 0x0:
			if (nnn == 0x0e0){		// 00E0 (clear screen)
				m_display[32][64] = {0};
				reset();
			}
			else if (nnn == 0x0ee){	// 00EE (return from call)
				m_PC = m_stack[--m_SP % 12];
			}
			break;
		case 0x1:
			// if 0x1NNN, jump to NNN
			m_PC = nnn - 2;			// decrement by 2 so next opcode is not skipped
			break;
		case 0x2:
			// Call subroutine at NNN
			m_stack[m_SP++ % 12] = (u16)m_PC;
			m_PC = nnn - 2;			// decrement by 2 so next opcode is not skipped
			break;
		case 0x3:
			// If Vx == NN, skip next instruction
			if(kk == m_V[x])
				m_PC += 2;
			break;
		case 0x4:
			// 4xkk - If Vx != NN, skip next instruction
			if(kk != m_V[x])
				m_PC += 2;
			break;
		case 0x5:
			// 5xy0 - If Vx == Vy, skip next instruction
			if(m_V[x] == m_V[y])
				m_PC += 2;
			break;
		case 0x6:
			// 6xkk - Set Vx = kk
			m_V[x] = kk;
			break;
		case 0x7:
			// 7xkk - Set Vx = Vx + kk
			m_V[x] += kk;
			break;
		case 0x8:
			if (n == 0x0 ) // 8xy0 - Set Vx = Vy.
				m_V[x] = m_V[y];
			else if (n == 0x1) // 8xy1 - Set Vx = Vx OR Vy.
				m_V[x] |= m_V[y];
			else if (n == 0x2) // 8xy2 - Set Vx = Vx AND Vy.
				m_V[x] &= m_V[y];
			else if (n == 0x3) // 8xy3 - Set Vx = Vx XOR Vy. 
				m_V[x] ^= m_V[y];
			else if (n == 0x4) { // 8xy4 - Set Vx = Vx + Vy, set VF = carry 
				m_V[0xf] = ((unsigned)m_V[x] + (unsigned)m_V[y] > 0xff)? 1: 0;
				m_V[x] += m_V[y];
			}
			else if (n == 0x5) { // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
				m_V[0xf] = (m_V[x] > m_V[y]) ? 1 : 0;
				m_V[x] -= m_V[y];
			}
			else if (n == 0x6) { // 8xy6 - Set Vx = Vx SHR 1. 
				m_V[0xf] = m_V[x]  & 1;
				m_V[x] >>= 1;
			}	
			else if (n == 0x7) { //  SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow
				m_V[0xf] = (m_V[y] > m_V[x]) ? 1 : 0;
				m_V[x] = m_V[y] - m_V[x];
			}
			else if (n == 0xe) { // 8xyE - Set Vx = Vx SHL 1.
				m_V[0xf] = (m_V[y] >> 7) & 0x1; 
				m_V[x] = m_V[y] << 1;
			}
			break;
		case 0x9:
			if (m_V[x] != m_V[y]) //9xy0 - Skip next instruction if Vx != Vy.
				m_PC += 2;
			break;
		case 0xa: // Set I = nnn.
			m_I = nnn;
			//std::cout << "I = " << (int) m_I << std::endl;
			break;
		case 0xb: // Bnnn - Jump to location nnn + V0.
			m_PC = nnn + m_V[0] - 2; // Decrement by 2 so next instr. is not skipped
			break;
		case 0xc: { // Cxkk - Set Vx = random byte AND kk
				m_V[x] = (rand() % 256) & kk ;
				break;
			}
		case 0xd:
		{
			// see also https://github.com/craigthomas/Chip8Python/blob/master/chip8/cpu.py#L670
			u8 height = n;
			m_V[0xf] = 0;
			for (u8 row = 0; row < height; row++) {
				u8 sprite = m_mem[m_I + row] ; // one row of the sprite
				//std::cout << "drawing " << std::hex << (int) sprite << std::endl;
				for(u8 col = 0; col < 8; col++) {
					// if this condition is true, we want to turn on a pixel
					if((sprite & 0x80) != 0) {
						// toggle current pixel and if it was previously on, set Vf to 1
						if (m_display[m_V[y] + row][m_V[x] + col] == 1)
							m_V[0xf] = 1;
						m_display[m_V[y] + row][m_V[x] + col] ^= 1;
					}
					// next bit
					sprite <<= 1;
				}
			}
			// redraw whole display
			renderAll(m_display);
			break;
		}
		case 0xf: 
			if (kk == 0x07) // Fx07 - Set Vx = delay timer value.
				m_V[x] = m_delayTimer;
			else if (kk == 0xa) // Fx0A - Wait for a key press, store the value of the key in Vx.
				;// TODO
			else if (kk == 0x15) // Fx15 - Set delay timer = Vx.
				m_delayTimer = m_V[x];
			else if (kk == 0x18) // Fx18 - Set sound timer = Vx.
				m_soundTimer = m_V[x];
			else if (kk == 0x1e)  {// Fx1E - Set I = I + Vx.
				m_V[0xf] = (m_I + m_V[x] > 0xfff) ? 1 : 0;
				m_I += m_V[x];
			}
			else if (kk == 0x29) // Fx29 - Set I = location of sprite for digit Vx
				m_I = m_V[x] * 5;  // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx
			else if (kk == 0x33) {//BCD representation of Vx in memory locations I, I+1, and I+2
				m_mem[(m_I+0)&0xFFF] = (m_V[x] % 1000) / 100;
				m_mem[(m_I+1)&0xFFF] = (m_V[x] % 100) / 10;
				m_mem[(m_I+2)&0xFFF] = m_V[x] % 10;
			}
			else if (kk == 0x55) { // Fx55 - Store registers V0 through Vx in memory starting at location I.
				for(unsigned xx = 0; xx <= x; xx++) {
					m_mem[m_I++ & 0xFFF] = m_V[xx];
				}
			} else if (kk == 0x65) { // Fx65 - Read registers V0 through Vx from memory starting at location I 
				for(unsigned xx = 0; xx <= x; xx++) {
					m_V[xx]= m_mem[m_I++ & 0xFFF];
				}
			}
			break;
	}
	// move to next instruction
	m_PC += 2;
	// Chip-8 runs at 60 Hz
	//std::this_thread::sleep_for(std::chrono::milliseconds(5));
	// update sound and delay timers at 60 Hz
	if (m_delayTimer > 0) 
		m_delayTimer--;
	if (m_soundTimer > 0) {
		m_soundTimer--;
		// TODO: beeping sound	(single frequency)
	}
	//std::cout << std::hex << (int)m_opcode << std::endl;
}


void Chip8::run(unsigned startingOffset) {
	m_PC = startingOffset;
	// the trick to stop the loop is when 2 consecutive bytes of free space (0xff) are encountered
	while ((m_mem[m_PC] != 0xff) || (m_mem[m_PC+1] != 0xff)) {
		Chip8::fetch();
		Chip8::decode();
		Chip8::exec();
	}
}
