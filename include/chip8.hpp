#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include "cfg_parser.hpp"
#include "keypad.hpp"
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
        void Exec(const opcode_t& opc);
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
        /** @brief Update the delay and sound timer. */
        void UpdateTimers();
        std::array<uint8_t, 0x1000> ram_;  // Main memory
        uint16_t PC_;                      // Program counter - points to current instruction
        std::array<uint8_t, 16> regs_;     // Arithmetic operation registers
        std::array<uint16_t, 12> stack_;   // Stack - stores addresses for subroutine calls
        uint16_t SP_;                      // Stack pointer
        uint16_t I_;                       // Index register - read and write in RAM
        std::array<uint8_t, ROWS*COLS> frame_buffer_; // Pixels to render (monochrome)
        /** The hardware clock - i.e. how many instructions the emulator can run per sec */
        unsigned freq_;
        /** If non zero, ticks down at 60 Hz */
        std::atomic<uint8_t> delay_timer_;
        /** If non zero, ticks down at 60 Hz. Should make the system beep is zero. */
        std::atomic<uint8_t> sound_timer_;
        /** Maps keys from a real keyboard to Chip8's keypad */
        std::unordered_map<char, uint8_t> keyboard2keypad_ = Keypad::keyboard2keypad;
        std::unordered_map<uint8_t, bool> pressed_keys_;
        
        std::atomic<bool> run_timers_;     // flag to start delay and sound timer thread
        std::atomic<bool> run_key_thread_; // flag to start keyboard listener thread
        std::thread timer_thread_;
        std::thread key_thread_;
        std::mutex mutex_key_press_;
        /** Running state - running/paused/stepping/stopped */ 
        std::atomic<int> state_;
        std::unique_ptr<CfgParser> cfg_parser_;
        /** Last key pressed by the actual keyboard */
        char kbd_pressed_key_;
};

#endif /* CHIP8_HPP */
