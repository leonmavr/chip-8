#include "chip8.hpp" 
#include "keyboard.hpp" 
#include "toot.h" 
#include "logger.hpp" 
#include <random>
#include <cstdlib>
#include <cassert>
#include <unordered_map>
#include <chrono>
#include <thread>


static unsigned execInsrPerSec = 0;


void Chip8::loadRom(const char* filename, unsigned offset) {
	std::ifstream infile(filename);
	if (!infile.good())
		throw std::runtime_error("ROM not found\n");

	// write to memory - adapted from https://bisqwit.iki.fi/jutut/kuvat/programming_examples/chip8/chip8.cc
	for(std::ifstream f(filename, std::ios::binary); f.good();) 
		m_mem[offset++ & 0xFFF] = f.get();
}


uint8_t Chip8::rand() {
	std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<uint8_t> dist(0, 255);
    return dist(rng);
}


inline void Chip8::fetch() {
	assert(m_PC < 0x1000);
	// copy the current byte of the program to the current instruction,
	// assuming little endian host
	m_opcode = m_mem[m_PC] << 8 | m_mem[m_PC+1];
}


inline void Chip8::decode() {
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


void Chip8::exec() {
	const auto& x = m_bitfields.x;
	const auto& y = m_bitfields.y;
	const auto& kk = m_bitfields.kk;
	const auto& nnn = m_bitfields.nnn;
	const auto& n = m_bitfields.n;

	auto& Vx = m_V[x];
	auto& Vy = m_V[y];
	auto& Vf = m_V[0xf];
	auto& I = m_I;
	auto& PC = m_PC;

	switch(m_bitfields.type) {
		case 0x0:
			if (nnn == 0x0e0){		// 00E0 (clear screen)
				//m_display[32][64] = {0};
				cls();
			}
			else if (nnn == 0x0ee){	// 00EE (return from call)
				PC = m_stack[--m_SP % 12];
			}
			break;
		case 0x1:
			// if 0x1NNN, jump to NNN
			PC = nnn - 2;			// decrement by 2 so next opcode is not skipped
			break;
		case 0x2:
			// Call subroutine at NNN
			m_stack[m_SP++ % 12] = (uint16_t)PC;
			PC = nnn - 2;			// decrement by 2 so next opcode is not skipped
			break;
		case 0x3:
			// If Vx == NN, skip next instruction
			if (kk == Vx)
				PC += 2;
			break;
		case 0x4:
			// 4xkk - If Vx != NN, skip next instruction
			if (kk != Vx)
				PC += 2;
			break;
		case 0x5:
			// 5xy0 - If Vx == Vy, skip next instruction
			if (Vx == Vy)
				PC += 2;
			break;
		case 0x6:
			// 6xkk - Set Vx = kk
			Vx = kk;
			break;
		case 0x7:
			// 7xkk - Set Vx = Vx + kk
			Vx += kk;
			break;
		case 0x8:
			switch(n) {
				case 0x0: // 8xy0 - Set Vx = Vy.
					Vx = Vy;
					break;
				case 0x1: // 8xy1 - Set Vx = Vx OR Vy.
					Vx |= Vy;
					break;
				case 0x2: // 8xy2 - Set Vx = Vx AND Vy.
					Vx &= Vy;
					break;
				case 0x3: // 8xy3 - Set Vx = Vx XOR Vy. 
					Vx ^= Vy;
					break;
				case 0x4: // 8xy4 - Set Vx = Vx + Vy, set VF = carry 
					Vf = ((unsigned)Vx + (unsigned)Vy > 0xff)? 1: 0;
					Vx += Vy;
					break;
				case 0x5: // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
					Vf = (Vx > Vy) ? 1 : 0;
					Vx -= Vy;
					break;
				case 0x6: // 8xy6 - Set Vx = Vx SHR 1. 
					Vf = Vx & 1;
					Vx >>= 1;
					break;
				case 0x7: //  SUBN Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow
					Vf = (Vy > Vx) ? 1 : 0;
					Vx = Vy - Vx;
					break;
				case 0xe: // 8xyE - Set Vx = Vx SHL 1.
					Vf = (Vy >> 7) & 0x1; 
					Vx = Vy << 1;
					break;
			}
			break;
		case 0x9:
			if (Vx != Vy) //9xy0 - Skip next instruction if Vx != Vy.
				PC += 2;
			break;
		case 0xa: // Set I = nnn.
			I = nnn;
			break;
		case 0xb: // Bnnn - Jump to location nnn + V0.
			PC = nnn + m_V[0] - 2; // Decrement by 2 so next instr. is not skipped
			break;
		case 0xc: { // Cxkk - Set Vx = random byte AND kk
			Vx = rand() & kk ;
			break;
		}
		case 0xd:
		{
			uint8_t height = n;
			Vf = 0;
			for (uint8_t row = 0; row < height; row++) {
				uint8_t sprite = m_mem[I + row] ; // one row of the sprite
				for(uint8_t col = 0; col < 8; col++) {
					// if this condition is true, we want to turn on a pixel
					if((sprite & 0x80) != 0) {
						// toggle current pixel and if it was previously on, set Vf to 1
						if (m_display[Vy + row][Vx + col] == 1)
							Vf = 1;
						m_display[Vy + row][Vx + col] ^= 1;
					}
					// next bit
					sprite <<= 1;
				}
			}
			// redraw whole display
			renderAll(m_display);
			break;
		}
		case 0xe:
			if (kk == 0x9e) { // skip next instruction if key the the value of Vx is pressed
				if (m_keypresses[Vx & 15])  {
					PC += 2;
				}
			}
			if (kk == 0xa1) {// skip next instruction if  key with the value of Vx  is not pressed
				if (m_keypresses[Vx & 15] == 0)  {
					PC += 2;
				}
			}
		case 0xf: 
			switch(kk) {
				case 0x07: // Fx07 - Set Vx = delay timer value.
					Vx = m_delayTimer;
					break;
				case 0x0a: // Fx0A - Wait for a key press, store the value of the key in Vx.
					Vx = Keyboard::getKeyPress();
					break;
				case 0x15: // Fx15 - Set delay timer = Vx.
					m_delayTimer = Vx;
					break;
				case 0x18: // Fx18 - Set sound timer = Vx.
					m_soundTimer = Vx;
					break;
				case 0x1e: // Fx1E - Set I = I + Vx.
					Vf = (I + Vx > 0xfff) ? 1 : 0;
					I += Vx;
					break;
				case 0x29: // Fx29 - Set I = location of sprite for digit Vx
					I = Vx * 5;  // The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx
					break;
				case 0x33: //BCD representation of Vx in memory locations I, I+1, and I+2
					m_mem[(I+0)&0xFFF] = (Vx % 1000) / 100;
					m_mem[(I+1)&0xFFF] = (Vx % 100) / 10;
					m_mem[(I+2)&0xFFF] = Vx % 10;
					break;
				case 0x55: // Fx55 - Store registers V0 through Vx in memory starting at location I.
					for(unsigned xx = 0; xx <= x; xx++)
						m_mem[I++ & 0xFFF] = m_V[xx];
					break;
				case 0x65: // Fx65 - Read registers V0 through Vx from memory starting at location I 
					for(unsigned xx = 0; xx <= x; xx++)
						m_V[xx] = m_mem[I++ & 0xFFF];
					break;
			}
		break;
		}

	if (m_clockSpeed == SPEED_NORMAL) {
		// decrement every 60 hz and play sound if necessary
		if (m_instrPerSec % 10 == 0)   {
			if (m_delayTimer > 0) 
				m_delayTimer--;
			if (m_soundTimer > 0) {
				m_soundTimer--;
				if (!m_mute)
					toot(700.0, 5); // freq (Hz), duration (ms)
			}
		}
	} else { // overclocked mode
		if (m_delayTimer > 0) 
			m_delayTimer--;
		if (m_soundTimer > 0)
			m_soundTimer--;
	}

	execInsrPerSec++;
	// move to next instruction
	m_PC += 2;
}


void Chip8::run(unsigned startingOffset) {
	std::chrono::high_resolution_clock::time_point t_start, t_end;
	int t_deltaUs;

	m_PC = startingOffset;
	// the trick to stop the loop is when 2 consecutive bytes of free space (0xff) are encountered
	while ((m_mem[m_PC] != 0xff) || (m_mem[m_PC+1] != 0xff)) {
		if (m_clockSpeed == SPEED_NORMAL)
			t_start = std::chrono::high_resolution_clock::now();

		// fetch-decode-exec defines the operation of Chip8
		Chip8::fetch();
		Chip8::decode();
		Chip8::getKeyPress();
		Chip8::exec();

		// compensate the fps every 10th of a second to make emulation smoother
		if ((!m_overclock) && (execInsrPerSec/10 >= m_instrPerSec/10)) {
			t_end = std::chrono::high_resolution_clock::now();
			t_deltaUs = (t_end - t_start)/std::chrono::milliseconds(1)*1000;
			std::this_thread::sleep_for(std::chrono::microseconds(
						static_cast<bool>(100000 > t_deltaUs) * (100000 - t_deltaUs)
					));
			execInsrPerSec = 0;
		}
		if (m_maxIter > 0) {
			if (Logger::getInstance().count() > m_maxIter) {
				Logger::getInstance().screendump(m_display); // dump screen to file
			return;
			}
		}
	}
}


void Chip8::init(const int& overclock, const int& instrPerSec, const int& maxIter, const int& mute) {
	// 1. Initialise special registers and memory
	m_SP = 0x0;
	m_PC = 0x200;
	m_I = 0x0;
	for (auto& m: m_mem)
		m = 0xff; // 0xff indicates free space
	m_delayTimer = 0x0;
	m_soundTimer = 0x0;

	// 2. Write font sprites to memory (locations 0x0 to 0x4f inclusive)
	// define font sprites - see https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#font
	std::vector<uint8_t> m_fontset = 
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
	// copy to m_mem
	unsigned fontOffset = 0x0;
	for (const uint8_t& element: m_fontset)
		m_mem[fontOffset++ & 0xFF] = element;

	// 3. config options (from IniReader)
	m_overclock = static_cast<bool>(overclock);
	m_instrPerSec = instrPerSec;
	m_maxIter = maxIter;
	m_mute = static_cast<bool>(mute);
}
