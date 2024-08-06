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
         * @brief Load a ROM from a filepath
         * @param file path to the ROM file
         */
        void LoadRom(const char* filename);
        /**
         * @brief Run the emulator after loading the ROM. It runs the
                  fetch-decode-execute cycle until the user interrupts it.
         */
        void Run();

    private:
        std::array<uint8_t, 0x1000> ram_;             // Whole memory
        std::array<uint8_t, 16> regs_;                // V (general) registers
        uint16_t SP_;                                 // Stack pointer
        uint16_t PC_;                                 // Program counter
        uint16_t I_;                                  // Index register
        std::array<uint8_t, ROWS*COLS> frame_buffer_;
        std::array<uint16_t, 12> stack_;

        /**
         * @brief Fetch the instruction that PC points to.
         * @return The current instruction as a 2-byte.
         */
        inline uint16_t Fetch() const;
        /**
         * @brief Decode the given instruction.
         * @param instr The 2-byte instruction to decode.
         * @returns The decoded instruction as an opcode structure.
         */
        inline opcode_t Decode(uint16_t instr) const;
        
        /**
         * @brief Execute the decoded opcode
         * @param opc The opcode to execute
         */
        void Exec(opcode_t opc);

        unsigned freq_;
        std::unordered_map<char, uint8_t> keyboard2keypad_ = {
            {'1', 0x1}, {'2', 0x2}, {'3', 0x3}, {'4', 0xC}, {'q', 0x4}, {'w', 0x5},
            {'e', 0x6}, {'r', 0xD}, {'a', 0x7}, {'s', 0x8}, {'d', 0x9}, {'f', 0xE},
            {'z', 0xA}, {'x', 0x0}, {'c', 0xB}, {'v', 0xF}
        };
        std::unordered_map<uint8_t, bool> key_states_;

        /** @brief Listen for a key press (without blocking the program).  */
        void ListenForKey();
        
        /**
         * @brief Wait for a key press (blocks the program).
         * @returns The pressed Chip8 keypad key.
         */
        uint8_t WaitForKey();
        /** @brief Clear the screen. */
        inline void Cls();
        /** @brief Render the entire screen. */
        void RenderFrame();

        std::atomic<uint8_t> delay_timer_;
        std::atomic<uint8_t> sound_timer_;
        std::atomic<bool> run_timers_;
        std::thread timer_thread_;
        std::mutex mutex_key_press_;
        std::thread key_thread_;
        std::atomic<bool> run_key_thread_;

        /** @brief Update the delay and sound timer. */
        void UpdateTimers();
        
        std::atomic<int> state_;
        std::unique_ptr<CfgParser> cfg_parser_;
        char kbd_pressed_key_;
};

#endif /* CHIP8_HPP */
