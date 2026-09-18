#ifndef CHIP8_HPP
#define CHIP8_HPP 

#include "cfg_parser.hpp"
#include "keypad.hpp"
#ifdef CHIP8_ENABLE_SOUND
#include "beeper.hpp"
#endif
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <cinttypes>
#include <limits>


#ifdef RUN_UNIT_TESTS
class Chip8Tester; // forward declaration of unit tester class
#endif

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
        /**
         * @param sound_enabled Start the sound thread. When false, no sound
         * thread is created at all, so the sound timer simply counts down
         * silently and no audio device is touched.
         */
        Chip8(bool sound_enabled = false);
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
        void Run(size_t max_iterations = std::numeric_limits<size_t>::max());
        /**
         * @brief Start the sound thread, if sound was not already enabled.
         *
         * The thread can't be started during construction when the caller only
         * learns about --sound after parsing argv. Safe to call more than once:
         * the thread is created at most once, and does nothing when the build
         * has no sound support.
         */
        void StartSound();
#ifdef RUN_UNIT_TESTS
        /** Unit tester class; has access to this class' data */
        friend class Chip8Tester;
#endif
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
         * @brief Wait for a fresh key press (blocks the program).
         * @returns The pressed Chip8 keypad key, or 0 if the emulator is stopping.
         */
        uint8_t WaitForKey();
        /**
         * @brief Release keys that have stopped being refreshed by the keyboard.
         *
         * A terminal reports auto-repeat, not key release, so a key is considered
         * held while events keep arriving and is released once they stop.
         */
        void ClearHeldKeys();
        /** @brief Clear the screen. */
        inline void Cls();
        /** @brief Update the phosphor overlay based on the previous frame. 
         * The phosphor overlay is a dimmed version of the previous frame
         * drawn before the current frame.
         */
        void UpdatePhosphorBuffer();
        /** @brief Render the entire screen. */
        void RenderFrame();
        /** @brief Build the whole frame as a UTF-8 string, without printing it. */
        std::string BuildFrame();
        /**
         * @brief Exit with an error if the terminal is too small for the frame.
         * Does nothing when the terminal size can't be determined.
         */
        void CheckTerminalFits();
        /** @brief Update the delay and sound timer. */
        void UpdateTimers();
#ifdef CHIP8_ENABLE_SOUND
        /**
         * @brief Beep while the sound timer is running, in its own thread.
         *
         * Drives the Beeper (ALSA) with a square wave; falls back to silence
         * when no audio device is available. Compiled out entirely when the
         * build has no sound support.
         */
        void PlaySound();
        /** Tone emitted for the sound timer, in Hz (defaults to A4). */
        unsigned beep_freq_hz_ = 440;
#endif
        std::array<uint8_t, 0x1000> ram_;  // Main memory
        uint16_t PC_;                      // Program counter - points to current instruction
        std::array<uint8_t, 16> regs_;     // Arithmetic operation registers
        std::array<uint16_t, 12> stack_;   // Stack - stores addresses for subroutine calls
        uint16_t SP_;                      // Stack pointer
        uint16_t I_;                       // Index register - read and write in RAM
        std::array<uint8_t, ROWS*COLS> frame_buffer_; // Pixels to render (monochrome)
        /** Previous full frame, used for to fade the recently changed pixels */
        std::array<uint8_t, ROWS*COLS> previous_frame_buffer_;
        /** Bitmask of pixels that changed from the previous frame - should be drawn dimly */
        std::array<uint8_t, ROWS*COLS> previous_buffer_diff_;
        /** The hardware clock - i.e. how many instructions the emulator can run per sec */
        unsigned freq_;
        /** If non zero, ticks down at 60 Hz */
        std::atomic<uint8_t> delay_timer_;
        /** If non zero, ticks down at 60 Hz. Should make the system beep if zero. */
        std::atomic<uint8_t> sound_timer_;
        /** Maps keys from a real keyboard to Chip8's keypad */
        const std::unordered_map<char, uint8_t>& keyboard2keypad_ = Keypad::keyboard2keypad;
        /** Whether each of the 16 Chip8 keys is currently held down */
        std::array<std::atomic<bool>, 16> key_is_pressed_;
        /**
         * Time (ms) of the last keyboard event seen for each key.
         * Heuristic for smooth (not instant) release. Used to
         * measure how long a key has been not pressed and once we 
         * exceed a timeout, the key is meant to be marked as released.
         */
        std::array<std::atomic<unsigned>, 16> key_last_seen_ms_;
        /**
         * Tally of rising edge (not held to held) for each key. 
         * Auto-repeat never updates it, so a newer value means 
         * fresh continuous press. Required for Fx0A instructions.
         */
        std::array<std::atomic<unsigned>, 16> keypress_edge_ctr_;
        /** Whether each key has shown auto-repeat since it was last pressed. */
        std::array<std::atomic<bool>, 16> key_is_autorepeat_;
        /** Running tally of all press edges, used to stamp key_press_seq_. */
        std::atomic<unsigned> key_press_counter_;

        // whether to start the delay and sound timer thread
        std::atomic<bool> run_timers_;
        // whether to start the key-listening thread
        std::atomic<bool> run_key_thread_;
        // whether to start the sound (beeper) thread
        std::atomic<bool> run_sound_thread_;
        std::mutex mutex_key_press_;
        /** Running state (running/paused/stepping/stopped) */ 
        std::atomic<int> state_;
        std::unique_ptr<CfgParser> cfg_parser_;
        /** Last key pressed by the actual keyboard */
        std::atomic<char> kbd_pressed_key_;
        // NOTE: threads must be started after their addressees
        // (atomic vars) are constructed
        std::thread timer_thread_;
        std::thread key_thread_;
        /** Created only when sound is enabled; empty otherwise. */
        std::thread sound_thread_;
#ifdef CHIP8_ENABLE_SOUND
        /** Emits the beep for the sound timer; only used by the sound thread. */
        Beeper beeper_;
#endif

        /** Whether to use SCHIP1.1's quirks: https://chip8.gulrak.net/ */
        bool use_quirks_;

};

#endif /* CHIP8_HPP */
