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
#include <cinttypes> // uint8_t, uint16_t

#define ROWS 32
#define COLS 64
#define ROM_OFFSET 0x200


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
        Chip8();
        ~Chip8();
        /**
         * @brief 				Load a rom from a filepath
         *
         * @param filename		File name of rom
         * @param offset		Starting offset in Chip8's memory. Rom data will be written to this offset onwards (towards higher addresses).
         * 						`offset` is part of the architecture to it should be left to the default value of 0x200	.
         */
        void LoadRom(const char* filename);
        /**
         * @brief 
         *
         * @param startingOffset Run the rom data that are starting an offset. Again, the standard offset for the rom is 0x200.
         */
        void Run();

    private:
        /* define the architecture */
        std::array<uint8_t, 0x1000> ram_;             // Whole memory
        std::array<uint8_t, 16> regs_;                // V (general) registers
        uint16_t SP_;                                 // Stack pointer
        uint16_t PC_;                                 // Program counter
        uint16_t I_;                                  // Index register
        std::array<uint8_t, ROWS*COLS> pixels_;
        std::array<uint16_t, 12> stack_;

        inline uint16_t Fetch() const;                // handles current instruction
        inline opcode_t Decode(uint16_t instr) const; // handles current instruction
        void Exec(opcode_t opc);                      // handles current instruction

        /* frequency - i.e. how many instructions cycles the machine can run her second */
        unsigned freq_;
        std::unordered_map<char, uint8_t> keyboard2keypad_ = {
            {'1', 0x1}, {'2', 0x2}, {'3', 0x3}, {'4', 0xC}, {'q', 0x4}, {'w', 0x5},
            {'e', 0x6}, {'r', 0xD}, {'a', 0x7}, {'s', 0x8}, {'d', 0x9}, {'f', 0xE},
            {'z', 0xA}, {'x', 0x0}, {'c', 0xB}, {'v', 0xF}
        };
        std::unordered_map<uint8_t, bool> key_states_;

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
        std::atomic<bool> stop_key_thread_;

        void UpdateTimers();
        
        std::atomic<int> state_;
        std::unique_ptr<CfgParser> cfg_parser_;
        char kbd_pressed_key_;
};

#endif /* CHIP8_HPP */
