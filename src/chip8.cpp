#include "chip8.hpp" 
#include "frontend.hpp" 
#include "cfg_parser.hpp" 
#include "term.h"
#include <random>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <termios.h>
#include <thread>
#include <mutex>
#include <fcntl.h>
#include <iostream>
#include <fstream>

#ifndef CHIP8_USE_PHOSPHOR
#define CHIP8_USE_PHOSPHOR 1
#endif

#define KEY_ESC 27
#define ERROR_POINTER() { Chip8::~Chip8(); throw std::runtime_error("Fatal: Invalid program or stack pointer!\n"); }

static struct termios orig_termios;

static void SetNonBlockingInput() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios tty = orig_termios;
    // do not return what's written, do not echo
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

static void ResetBlockingInput() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    fcntl(STDIN_FILENO, F_SETFL, 0);
    TPRINT_SHOW_CURSOR();
    TPRINT_LEAVE_ALT_SCREEN();
    fflush(stdout);
}

static unsigned NowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

/**
 * Release a key this long after its last event once auto-repeat has been
 * observed. Auto-repeat fires every ~33 ms, so 100 ms spans a whole repeat
 * period with margin and keeps release detection snappy.
 */
static constexpr unsigned KEY_REPEAT_TIMEOUT_MS = 100;

/**
 * Release a key this long after a fresh press when auto-repeat has NOT yet been
 * observed. Terminals wait ~250 ms before the first auto-repeat, so a shorter
 * timeout would spuriously release a key that is simply being held. 350 ms
 * covers that initial delay with margin.
 */
static constexpr unsigned KEY_FIRST_PRESS_TIMEOUT_MS = 120;


Chip8::Chip8(bool sound_enabled):
    ram_{},
    PC_(ROM_OFFSET),
    regs_{},
    stack_{},
    SP_(0x00),
    I_(0x000),
    frame_buffer_{},
    previous_frame_buffer_{},
    previous_buffer_diff_{},
    freq_(350),
    delay_timer_(0x00),
    sound_timer_(0x00),
    key_is_pressed_{},
    key_last_seen_ms_{},
    keypress_edge_ctr_{},
    key_is_autorepeat_{},
    key_press_counter_(0),
    run_timers_(true),
    run_key_thread_(true),
    run_sound_thread_(sound_enabled),
    state_(STATE_RUNNING),
    cfg_parser_(nullptr),
    kbd_pressed_key_('\0'),
    timer_thread_(std::thread(&Chip8::UpdateTimers, this)),
    key_thread_(std::thread(&Chip8::ListenForKey, this)),
    use_quirks_(true)
{
    // preload memory with sprites 
    constexpr std::array<uint8_t, 80> font_sprites = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */ 0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
        0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */ 0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
        0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */ 0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
        0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */ 0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
        0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */ 0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
        0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */ 0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
        0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */ 0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
        0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */ 0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
    };
    std::copy(std::begin(font_sprites), std::end(font_sprites), std::begin(ram_));

    // initialize key states
    for (size_t i = 0; i < key_is_pressed_.size(); ++i) {
        key_is_pressed_[i] = false;
        key_last_seen_ms_[i] = 0;
        keypress_edge_ctr_[i] = 0;
        key_is_autorepeat_[i] = false;
    }
    previous_frame_buffer_.fill(0);
    previous_buffer_diff_.fill(0);
    SetNonBlockingInput();
    TPRINT_ENTER_ALT_SCREEN();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    TPRINT_HIDE_CURSOR();
    fflush(stdout);
}

Chip8::~Chip8 () {
    run_timers_ = false;
    if (timer_thread_.joinable())
        timer_thread_.join();
    run_key_thread_ = false;
    if (key_thread_.joinable())
        key_thread_.join();
    run_sound_thread_ = false;
    if (sound_thread_.joinable())
        sound_thread_.join();
    ResetBlockingInput();
};

void Chip8::StartSound() {
#ifdef CHIP8_ENABLE_SOUND
    if (sound_thread_.joinable())
        return; // already running
    run_sound_thread_ = true;
    sound_thread_ = std::thread(&Chip8::PlaySound, this);
#endif
}

void Chip8::LoadRom(const char* filename) {
    std::ifstream infile(filename);
    if (!infile.good())
        throw std::runtime_error("ROM not found\n");
    // Write to memory
    infile.read(reinterpret_cast<char*>(&ram_[ROM_OFFSET & 0xFFF]), 0xFFF - ROM_OFFSET);
    infile.close();

    const size_t pos_last_dot = std::string(filename).find_last_of(".");
    std::string cfg_filename = std::string(filename).substr(0, pos_last_dot) + ".cfg" ;
    std::cout << cfg_filename << std::endl;
    cfg_parser_ = std::make_unique<CfgParser>(cfg_filename);
    freq_ = cfg_parser_->frequency();
    use_quirks_ = cfg_parser_->quirks();
}

void Chip8::Run(size_t max_iterations) {
    CheckTerminalFits();
    // clock functions - get time and sleep (milliseconds) 
    auto now_ms = []() -> unsigned {
        auto now = std::chrono::system_clock::now();
        auto since_epoch = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count();
    };
    auto sleep_ms = [](unsigned ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    };
    // t0 and t1 enforce the loop to cycle at frequency `freq_`
    unsigned t0 = now_ms(), t1 = now_ms();
    // t0_render and t1_render enforce the renderer to flush at 60 Hz 
    unsigned t0_render = now_ms(), t1_render = now_ms();
    /** Throttles the instructions at `freq_` instructions per second. It checks
     * how many instructions have run every 1/20 sec. If more than `freq_/20`, 
     * stall the loop until 1/20 of a sec has ellapsed. */
    static unsigned instr_per_50ms = 0;

    size_t iteration = 0;
    while (iteration < max_iterations) {
        if (state_ == STATE_PAUSED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            RenderFrame();
            continue;
        } else if (state_ == STATE_STEPPING) {
            kbd_pressed_key_ = '\0';
            while (kbd_pressed_key_ != 'S' && kbd_pressed_key_ != 'R' && kbd_pressed_key_ != 'P' && kbd_pressed_key_ != 'Q') {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if (kbd_pressed_key_ == 'P') {
                    state_ = STATE_PAUSED;
                    break;
                } else if (kbd_pressed_key_ == 'R') {
                    state_ = STATE_RUNNING;
                    break;
                } else if (kbd_pressed_key_ == KEY_ESC) {
                    state_ = STATE_STOPPED;
                    break;
                }
            }
        } else if (state_ == STATE_STOPPED) {
            break;
        }

        const uint16_t instr = Fetch();
        const opcode_t opc = Decode(instr);
        Exec(opc);
        
        t1_render = now_ms();
        if (t1_render - t0_render > 16) {
            RenderFrame();
            t0_render = t1_render;
        }

        if (instr_per_50ms++ % (freq_/20) == 0) {
            t1 = now_ms();
            if (t1 - t0 < 50)
                sleep_ms(50 - (t1 - t0));
            t0 = t1;
        }
        PC_ += 2;
        iteration++;
    }
}

inline uint16_t Chip8::Fetch() const {
   /*
    * Read two consecutive bytes from RAM and join them into a 16-bit instruction.
    * Start to read where PC is currently pointing. Return a 16-bit field containing
    *  [mem[PC] mem[PC+1]]. Here's how it's done e.g. for the 0x00E0 (CLS) instruction.
    *
    * RAM bytes:        Instruction
    *                   (destination)
    *  @PC  @PC+1
    *   |     | 
    *   v     v         <-16 bits->
    * +----+----+       +---------+
    * | 00 | E0 |       |         |
    * +----+----+       +---------+
    *
    * 1. dest =    | 2. dest <<= 8   | 3. dest |= 
    *    mem[PC]   |                 |    mem[PC+1]
    * +---------+  |    +---------+  |    +---------+
    * |      00 |  |    | 00      |  |    | 00   e0 |
    * +---------+  |    +---------+  |    +---------+
    */
    uint16_t instr = ram_[PC_] << 8;
    instr |= ram_[PC_ + 1];
    return instr;
}

inline opcode_t Chip8::Decode(uint16_t instr) const {
    /*
     * A Chip8 instruction always takes 2 bytes, or 4 nibbles (half-bytes).
     * N0 denotes the highest address nibble (leftmost), N1 the second-highest,
     * N2 the third and N3 the fourth.
     * An instruction encodes the following 6 fields in an overlapping region;
     * prefix, x operand, y operand, nnn, nn, n. These fields are  decoded into
     * a structure. Not all of them are used together. For example, if either x
     * or y is used, then nn is not * used.
     *
     * MSB (@0xFE)     LSB (@0x0)
     *  |                   |
     *  v                   v
     *  +----+----+----+----+
     *  | N0 | N1 | N2 | N3 | (N: nibble = 4 bits)
     *  +----+----+----+----+
     *  <---->    |    |    |
     *  prefix    |    |    |
     *       <---->    |    |
     *       | x  |    |    |
     *       |    <---->    |
     *       |    | y  |    |
     *       |    |    <---->
     *       |    |      n  | 
     *       |    <--------->
     *       |        nn    |
     *       <-------------->
     *              nnn 
     *
     * References:
     * -----------
     * 1. https://johnearnest.github.io/Octo/docs/chip8ref.pdf
     */
    opcode_t decoded;
    decoded.prefix = (instr >> 12) & 0x00f;
    decoded.n      = instr         & 0x00f;
    decoded.x      = (instr >> 8)  & 0x00f;
    decoded.y      = (instr >> 4)  & 0x00f;
    decoded.nn     = instr         & 0x0ff;
    decoded.nnn    = instr         & 0xfff;
    return decoded;
}


void Chip8::Exec(const opcode_t& opc) {
    const auto x = opc.x;
    const auto y = opc.y;
    const auto n = opc.n;
    const auto nn = opc.nn;
    const auto nnn = opc.nnn;
    const auto prefix = opc.prefix;
    // aliases for registers and pointers
    auto& Vx = regs_[x];
    auto& Vy = regs_[y];
    auto& Vf = regs_[0xf]; // detects overflow, e.g. in addition
    auto& I = I_;
    auto& PC = PC_;
    auto& SP = SP_;
    // initialise RNG with a seed 
    static std::mt19937 seed(std::random_device{}());
    static std::uniform_int_distribution<uint8_t> rng(0, 255); // inclusive

#define EXEC_INSTRUCTION \
    /* assembly    , condition                     , instruction(s) */                        \
    X("ERR"        , (prefix == 0x0 && nnn == 0x0ee && SP == 0)     ||                        \
                     ((prefix == 0x1 || prefix == 0x2) && nnn == 0) ||                        \
                     (prefix == 0x2 &&  SP >= stack_.size() - 1)                              \
                                                   , ERROR_POINTER(); )                       \
    X("CLS"        , prefix == 0x0 && nnn == 0x0e0 , Cls();)                                  \
    X("RET"        , prefix == 0x0 && nnn == 0x0ee , PC = stack_[--SP];)                      \
    X("JP nnn"     , prefix == 0x1                 , PC = nnn - 2;)                           \
    X("CALL nnn"   , prefix == 0x2                 , stack_[SP++] = PC; PC = nnn - 2;)        \
    X("SE Vx nn"   , prefix == 0x3 && nn == Vx     , PC += 2;)                                \
    X("SNE Vx nn"  , prefix == 0x4 && nn != Vx     , PC += 2;)                                \
    X("SE Vx Vy"   , prefix == 0x5 && n == 0x0                                                \
                                   && Vx == Vy     , PC += 2;)                                \
    X("LD Vx nn"   , prefix == 0x6                 , Vx = nn;)                                \
    X("ADD Vx nn"  , prefix == 0x7                 , Vx += nn;)                               \
    X("LD Vx Vy"   , prefix == 0x8 && n == 0x0     , Vx = Vy;)                                \
    X("OR Vx Vy"   , prefix == 0x8 && n == 0x1     , Vx |= Vy;)                               \
    X("AND Vx Vy"  , prefix == 0x8 && n == 0x2     , Vx &= Vy;)                               \
    X("XOR Vx Vy"  , prefix == 0x8 && n == 0x3     , Vx ^= Vy;)                               \
    X("ADD Vx Vy"  , prefix == 0x8 && n == 0x4     , uint16_t sum = Vx + Vy; Vx = sum & 0xFF; \
                                                     Vf = sum > 0xFF;)                        \
    X("SUB Vx Vy"  , prefix == 0x8 && n == 0x5     , Vf = Vx >= Vy; Vx = (Vx - Vy) & 0xFF;)   \
    X("SHR Vx Vy"  , prefix == 0x8 && n == 0x6     , Vf = Vx & 0x1;                           \
                                                     if (use_quirks_)                         \
                                                         Vx >>= 1;                            \
                                                     else                                     \
                                                         Vx = Vy >> 1;)                       \
    X("SUBN Vx Vy" , prefix == 0x8 && n == 0x7     , Vf = Vy >= Vx; Vx = (Vy - Vx) & 0xFF;)   \
    X("SHL Vx Vy"  , prefix == 0x8 && n == 0xe     , Vf = (Vx >> 7) & 0x1;                    \
                                                     if (use_quirks_)                         \
                                                         Vx = (Vx << 1) & 0xFF;               \
                                                     else                                     \
                                                         Vx = (Vy << 1) & 0xFF;)              \
    X("SNE Vx Vy"  , prefix == 0x9 && n == 0x0                                                \
                                   && Vx != Vy     , PC += 2;)                                \
    X("LD I nnn"   , prefix == 0xa                 , I = nnn;)                                \
    X("JP V0 nnn"  , prefix == 0xb                 , PC = nnn + regs_[0] - 2;)                \
    X("RND Vx nn"  , prefix == 0xc                 , Vx = rng(seed) & nn;)                    \
    X("DRW Vx Vy n", prefix == 0xd,                                   \
        do {                                                          \
            const auto x0 = Vx;                                       \
            const auto y0 = Vy;                                       \
            Vf = 0;                                                   \
            for (uint8_t row = 0; row < n; ++row) {                   \
                uint8_t sprite = ram_[I + row];                       \
                for (uint8_t col = 0; col < 8; ++col, sprite <<= 1) { \
                    if (sprite & 0x80) {                              \
                        size_t x = x0 + col;                          \
                        size_t y = y0 + row;                          \
                        if (use_quirks_) {                            \
                            x %= COLS;                                \
                            y %= ROWS;                                \
                        }                                             \
                        if (x < COLS && y < ROWS) {                   \
                            size_t index = y * COLS + x;              \
                            Vf |= frame_buffer_[index];               \
                            frame_buffer_[index] ^= 1;                \
                        }                                             \
                    }                                                 \
                }                                                     \
            }                                                         \
        } while(0); )                                                 \
    X("SKP Vx"     , prefix == 0xe && nn == 0x9e   , if ( key_is_pressed_[Vx & 0xF]) PC += 2;)  \
    X("SKNP Vx"    , prefix == 0xe && nn == 0xa1   , if (!key_is_pressed_[Vx & 0xF]) PC += 2;)  \
    X("LD Vx DT"   , prefix == 0xf && nn == 0x07   , Vx = delay_timer_;)                        \
    X("LD Vx k"    , prefix == 0xf && nn == 0x0a   , Vx = WaitForKey();)                        \
    X("LD DT Vx"   , prefix == 0xf && nn == 0x15   , delay_timer_ = Vx;)                        \
    X("LD ST Vx"   , prefix == 0xf && nn == 0x18   , sound_timer_ = Vx;)                        \
    X("ADD I Vx"   , prefix == 0xf && nn == 0x1e   , I += Vx;)                                  \
    X("LD F Vx"    , prefix == 0xf && nn == 0x29   , I = Vx * 5;)                               \
    X("LD B Vx"    , prefix == 0xf && nn == 0x33   , ram_[(I + 0) & 0xFFF] = (Vx % 1000) / 100; \
                                                     ram_[(I + 1) & 0xFFF] = (Vx % 100) / 10;   \
                                                     ram_[(I + 2) & 0xFFF] = Vx % 10;)          \
    X("LD [I] Vx"  , prefix == 0xf && nn == 0x55   , for (unsigned xx = 0; xx <= x; xx++)       \
                                                         ram_[I++ & 0xFFF] = regs_[xx];         \
                                                     if (!use_quirks_) I += x + 1;)             \
    X("LD Vx [I]"  , prefix == 0xf && nn == 0x65   , for (unsigned xx = 0; xx <= x; xx++)       \
                                                         regs_[xx] = ram_[I++ & 0xFFF];         \
                                                     if (!use_quirks_) I += x + 1;)

    #define X(assembly, condition, instructions) if (condition) { instructions; break; }
    do {
        EXEC_INSTRUCTION
    } while(0);
    #undef X
    #undef EXEC_INSTRUCTION
}

void Chip8::ListenForKey() {
    while (run_key_thread_) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 25000;

        int success = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        if (success > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) != 1)
                continue;
            kbd_pressed_key_ = c;
            {
                std::lock_guard<std::mutex> lock(mutex_key_press_);
                enum {MAX_FREQ = 2000, MIN_FREQ = 50, FREQ_STEP = 50};
                if (kbd_pressed_key_ == 'S') state_ = STATE_STEPPING;
                else if (kbd_pressed_key_ == 'R') state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == 'P' && state_ != STATE_PAUSED) state_ = STATE_PAUSED;
                else if (kbd_pressed_key_ == 'P' && state_ == STATE_PAUSED) state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == KEY_ESC) state_ = STATE_STOPPED;
                else if (kbd_pressed_key_ == '+' && freq_ < MAX_FREQ) freq_ += FREQ_STEP;
                else if (kbd_pressed_key_ == '-' && freq_ > MIN_FREQ) freq_ -= FREQ_STEP;
                else if (kbd_pressed_key_ == 'Q') use_quirks_ = !use_quirks_;

                auto it = keyboard2keypad_.find(kbd_pressed_key_);
                if (it != keyboard2keypad_.end()) {
                    const size_t k = it->second;
                    const unsigned now = NowMs();
                    // record rising edge (was not pressed but presed now)
                    const bool was_pressed = key_is_pressed_[k].load();
                    if (!was_pressed) {
                        keypress_edge_ctr_[k] = ++key_press_counter_;
                    } else {
                        // A quick repeat while already held marks key as auto-repeat
                        // so this key can later be manually (exceeds timeout) released
                        const unsigned prev_ms = key_last_seen_ms_[k].load();
                        if (prev_ms != 0 && now - prev_ms < KEY_REPEAT_TIMEOUT_MS)
                            key_is_autorepeat_[k] = true;
                    }
                    key_last_seen_ms_[k] = now;
                    key_is_pressed_[k] = true;
                }
            }
        }
        // Clears keys that have been held for too long without auto-repeat,
        // or clears keys that have been held for too long after auto-repeat
        // has been observed
        ClearHeldKeys();
    }
}


void Chip8::ClearHeldKeys() {
    const unsigned now = NowMs();
    for (size_t i = 0; i < key_is_pressed_.size(); ++i) {
        if (!key_is_pressed_[i].load())
            continue;
        const unsigned timeout = key_is_autorepeat_[i].load()
                                     ? KEY_REPEAT_TIMEOUT_MS
                                     : KEY_FIRST_PRESS_TIMEOUT_MS;
        // Silence longer than the timeout; infer the key was released
        if (now - key_last_seen_ms_[i].load() > timeout) {
            key_is_pressed_[i] = false;
            key_is_autorepeat_[i] = false;
        }
    }
}


uint8_t Chip8::WaitForKey() {
    // Wait for a press that begins after this call
    // Auto-repeat on a key already held is not a new
    // press, so holding a key does not satisfy consecutive Fx0A instructions.

    // snapshot of key press sequence counter
    const unsigned seq_at_entry = key_press_counter_.load();
    while (run_key_thread_) {
        // keys are atomic so use load() to read them w/o locking the mutex
        for (size_t k = 0; k < key_is_pressed_.size(); ++k) {
            if (key_is_pressed_[k].load() &&
                keypress_edge_ctr_[k].load() > seq_at_entry) {
                return static_cast<uint8_t>(k);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return 0;
}

inline void Chip8::Cls() {
    frame_buffer_.fill(0);
    previous_frame_buffer_.fill(0);
    previous_buffer_diff_.fill(0);
    TPRINT_CLEAR();
    fflush(stdout);
}

void Chip8::UpdatePhosphorBuffer() {
#if CHIP8_USE_PHOSPHOR
    for (size_t i = 0; i < frame_buffer_.size(); ++i) {
        const bool prev_on = previous_frame_buffer_[i] != 0;
        const bool cur_on = frame_buffer_[i] != 0;
        previous_buffer_diff_[i] = (prev_on && !cur_on) ? 1 : 0;
        previous_frame_buffer_[i] = cur_on ? 1 : 0;
    }
#else
    previous_frame_buffer_.fill(0);
    previous_changed_buffer_.fill(0);
#endif
}

std::string Chip8::BuildFrame() {
    std::string horizontal = "";
    for (int i = 0; i < COLS; i++)
        horizontal += u8"\u2500";
    const std::string border_up = u8"\u250C" + horizontal + u8"\u2510" + "\n";
    const std::string border_down = u8"\u2514" + horizontal + u8"\u2518" + "\n";
    std::string pixels = border_up;
    for (size_t row = 0; row < ROWS; ++row) {
        // Build the row as a UTF-8 string. 
        // A lit pixel is drawn as a solid block, and previously ON
        // pixels are drawn as shaded boxes
        std::string frame_row;
        frame_row.reserve(COLS * 3);
        for (size_t col = 0; col < COLS; ++col) {
            const size_t index = row * COLS + col;
#if CHIP8_USE_PHOSPHOR
            if (frame_buffer_[index] != 0) {
                frame_row += u8"\u2588"; // solid box
            } else if (previous_buffer_diff_[index] != 0) {
                frame_row += u8"\u2591"; // shaded box
            } else {
                frame_row += " ";
            }
#else
            frame_row += (frame_buffer_[index] != 0) ? u8"\u2588" : " ";
#endif
        }
        const std::string border_left_right = u8"\u2502";
        pixels += border_left_right + frame_row + border_left_right;
        pixels += "\n";
    }
    pixels += border_down;
    // debugger and keyboard controls
    Frontend::WriteRegs(pixels, regs_);
    Frontend::WriteI(pixels, I_);
    Frontend::WritePC(pixels, PC_);
    Frontend::WriteSP(pixels, SP_);
    Frontend::WriteStack(pixels, stack_);
    int line_num = 10;
    Frontend::WriteRight(pixels, line_num++, "[P]ause/resume [S]tep [R]un [Esc]ape\n");
    const std::string quirks_state = (use_quirks_) ? "ON " : "OFF";
    Frontend::WriteRight(pixels, line_num++, "[Q]uirks: " + quirks_state + "\n");
    Frontend::WriteRight(pixels, line_num++, "[-] " + std::to_string(freq_) + " Hz [+]  \n");
    line_num++;
    for (const auto& key_descr: cfg_parser_->key_descrs()) {
        std::string key = key_descr.first;
        std::string descr = key_descr.second;
        Frontend::WriteRight(pixels, line_num++, "[" + key + "] " + descr + "\n");
    }

    // Actual keyboard keys in a 4x4 grid, highlighted if pressed
    std::array<char, 16> actual_keys{};
    for (const auto& [actual_key, chip8_key] : keyboard2keypad_) {
        actual_keys[chip8_key] = actual_key;
    }

    auto render_key_cell = [&](size_t idx) {
        const char actual_key = actual_keys[idx];
        const bool is_pressed = key_is_pressed_[idx].load();
        const std::string key_text = std::string(1, actual_key);
        if (is_pressed) {
            return std::array<std::string, 3>{
                "╔═╗",
                "║" + key_text + "║",
                "╚═╝"
            };
        }
        return std::array<std::string, 3>{
            "┌─┐",
            "│" + key_text + "│",
            "└─┘"
        };
    };

    Frontend::WriteRight(pixels, line_num++, "\n");
    for (size_t row = 0; row < 4; ++row) {
        std::string top_line, mid_line, bot_line;
        for (size_t col = 0; col < 4; ++col) {
            const auto cell = render_key_cell(row * 4 + col);
            top_line += cell[0];
            mid_line += cell[1];
            bot_line += cell[2];
            if (col != 3) {
                top_line += ' ';
                mid_line += ' ';
                bot_line += ' ';
            }
        }
        Frontend::WriteRight(pixels, line_num++, top_line + "\n");
        Frontend::WriteRight(pixels, line_num++, mid_line + "\n");
        Frontend::WriteRight(pixels, line_num++, bot_line + "\n");
    }

    return pixels;
}

void Chip8::RenderFrame() {
    UpdatePhosphorBuffer();
    TPRINT_GOTO_TOPLEFT();
    std::cout << BuildFrame() << "\n" << std::flush;
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void Chip8::CheckTerminalFits() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0 || ws.ws_col == 0 || ws.ws_row == 0)
        return; // size unknown; don't block the program

    // Measure the frame that would be rendered,
    // counting one column per UTF-8 character and 
    // one row per '\n'.
    const std::string frame = BuildFrame();
    size_t width = 0, height = 1, line_width = 0;
    for (size_t i = 0; i < frame.size();) {
        const unsigned char c = frame[i];
        if (c == '\n') {
            if (line_width > width) width = line_width;
            line_width = 0;
            ++height;
            i += 1;
        } else if (c < 0x80) { ++line_width; i += 1; }
        else if ((c >> 5) == 0x6) { ++line_width; i += 2; }
        else if ((c >> 4) == 0xe) { ++line_width; i += 3; }
        else { ++line_width; i += 4; }
    }
    if (line_width > width) width = line_width;

    if (width > ws.ws_col || height > ws.ws_row) {
        ResetBlockingInput(); // undo raw mode + alt screen before reporting
        std::cerr << "Error: terminal too small to render this program.\n"
                  << "  needs at least " << width << " cols x " << height << " rows,\n"
                  << "  but this terminal is " << (unsigned) ws.ws_col
                  << " cols x " << (unsigned) ws.ws_row << " rows.\n";
        std::exit(EXIT_FAILURE);
    }
}

void Chip8::UpdateTimers() {
    while (run_timers_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        if (delay_timer_ > 0) --delay_timer_;
        if (sound_timer_ > 0) --sound_timer_;
    }
}

#ifdef CHIP8_ENABLE_SOUND
void Chip8::PlaySound() {
    while (run_sound_thread_) {
        const bool beeping = sound_timer_ > 0;
        beeper_.SetTone(beeping, beep_freq_hz_);
    }
}
#endif
