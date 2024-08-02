#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include "cfg_parser.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <algorithm> // fill()
#include <cinttypes> // uint8_t, uint16_t
#include <bits/stdc++.h>

#define ROWS 32
#define COLS 64


typedef struct opcode_t {
    uint8_t prefix : 4; 
    uint8_t x : 4; 
    uint8_t y : 4; 
    uint8_t n : 4; 
    uint8_t nn : 8; 
    uint16_t nnn : 12; 
} opcode_t;

enum {
    STATE_RUNNING = 0,
    STATE_STEPPING,
    STATE_PAUSED,
    STATE_STOPPED,
};

class Chip8 {
    public:
        // don't forget to initialise constant members 
        Chip8 (std::string fnameIni);
        ~Chip8 ();
        /**
         * @brief 				Load a rom from a filepath
         *
         * @param filename		File name of rom
         * @param offset		Starting offset in Chip8's memory. Rom data will be written to this offset onwards (towards higher addresses).
         * 						`offset` is part of the architecture to it should be left to the default value of 0x200	.
         */
        void LoadRom(const char* filename, unsigned offset = 0x200);
        /**
         * @brief 
         *
         * @param startingOffset Run the rom data that are starting an offset. Again, the standard offset for the rom is 0x200.
         */
        void Run(unsigned startingOffset = 0x200);

    private:
        /* define the architecture */
        std::array<uint8_t, 0x1000> m_mem {};		// Whole memory
        std::array<uint8_t, 16> m_V {};				// V (general) registers
        uint8_t m_delayTimer, m_soundTimer;			// timers
        uint16_t m_SP;								// Stack pointer
        uint16_t m_PC;								// Program counter
        uint16_t m_I;								// Index register
        unsigned m_clockSpeed;						// clock speed (enum); normal or overclocked
        uint8_t m_display[32][64];
        std::array<uint8_t, ROWS*COLS> pixels_;

        std::array<uint16_t, 12>m_stack;			// stack
        std::vector<uint8_t> m_fontset;				// font sprites
        uint8_t Rand();								// Chip8 has a random number generator 

        uint16_t Fetch();                           // handles current instruction
        opcode_t Decode(uint16_t instr);            // handles current instruction
        void Exec(opcode_t opc);                    // handles current instruction

        void Init();								// initialises memory, registers, and configs

        /* frequency - i.e. how many instructions cycles the machine can run her second */
        unsigned freq_;
        std::unordered_map<char, uint8_t> keyboard2keypad_ = {
            {'1', 0x1}, {'2', 0x2}, {'3', 0x3}, {'4', 0xC}, {'q', 0x4}, {'w', 0x5},
            {'e', 0x6}, {'r', 0xD}, {'a', 0x7}, {'s', 0x8}, {'d', 0x9}, {'f', 0xE},
            {'z', 0xA}, {'x', 0x0}, {'c', 0xB}, {'v', 0xF}
        };
        const bool pressed = false;
        std::unordered_map<char, uint8_t> key_states_ = {
            {0x1, pressed}, {0x2, pressed}, {0x3, pressed}, {0xC, pressed},
            {0x4, pressed}, {0x5, pressed}, {0x6, pressed}, {0xD, pressed},
            {0x7, pressed}, {0x8, pressed}, {0x9, pressed}, {0xE, pressed},
            {0xA, pressed}, {0x0, pressed}, {0xB, pressed}, {0xF, pressed}
        };

        /* wait for key - non blocking */
        void PressKey();
        /* wait for key - blocking */
        uint8_t WaitForKey();

        void Cls();
        
        void RenderAll();

        std::atomic<uint8_t> delay_timer_;
        std::atomic<uint8_t> sound_timer_;
        std::atomic<bool> run_timers_;
        std::thread timer_thread_;
        std::mutex key_states_mutex_;
        std::thread key_thread_;
        bool stop_key_thread_ = false;

        void UpdateTimers();
        
        std::atomic<int> state_;
        std::unique_ptr<CfgParser> cfg_parser_;
        char kbd_pressed_key_;
};

#endif /* CHIP8_HPP */
