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
}


Chip8::Chip8():
    ram_{},
    PC_(ROM_OFFSET),
    regs_{},
    stack_{},
    SP_(0x00),
    I_(0x000),
    frame_buffer_{},
    freq_(350),
    delay_timer_(0x00),
    sound_timer_(0x00),
    run_timers_(true),
    run_key_thread_(true),
    timer_thread_(std::thread(&Chip8::UpdateTimers, this)),
    key_thread_(std::thread(&Chip8::ListenForKey, this)),
    state_(STATE_RUNNING),
    cfg_parser_(nullptr),
    kbd_pressed_key_('\0'),
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

    constexpr bool is_pressed = false;
    for (size_t i = 0x0; i < 0xF; ++i)
        pressed_keys_[i] = is_pressed;
    SetNonBlockingInput();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    TPRINT_HIDE_CURSOR();
}

Chip8::~Chip8 () {
    run_timers_ = false;
    if (timer_thread_.joinable())
        timer_thread_.join();
    run_key_thread_ = false;
    if (key_thread_.joinable())
        key_thread_.join();
    ResetBlockingInput();
};

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
    auto t_keyboard_start = std::chrono::high_resolution_clock::now();
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
            t1_render = t0_render;
        }
        

        const auto t_keyboard_end = std::chrono::high_resolution_clock::now();
        const unsigned dt_keyboard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_keyboard_end - t_keyboard_start).count();
        if (dt_keyboard_ms >= 100) {
            std::lock_guard<std::mutex> lock(mutex_key_press_);
            for (auto& pair : pressed_keys_)
                pair.second = false;
            t_keyboard_start = t_keyboard_end;
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
    X("SKP Vx"     , prefix == 0xe && nn == 0x9e   , if ( pressed_keys_[Vx & 0xF]) PC += 2;)    \
    X("SKNP Vx"    , prefix == 0xe && nn == 0xa1   , if (!pressed_keys_[Vx & 0xF]) PC += 2;)    \
    X("LD Vx DT"   , prefix == 0xf && nn == 0x07   , Vx = delay_timer_;)                        \
    X("LD Vx k"    , prefix == 0xf && nn == 0x0a   , Vx = WaitForKey();)                        \
    X("LD DT Vx"   , prefix == 0xf && nn == 0x15   , delay_timer_ = Vx;)                        \
    X("LD ST Vx"   , prefix == 0xf && nn == 0x18   , sound_timer_ = Vx;)                        \
    X("ADD I Vx"   , prefix == 0xf && nn == 0x1e   , I += Vx;)                                  \
    X("LD F Vx"    , prefix == 0xf && nn == 0x29   , I = (Vx & 0xF) * 5;)                       \
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
            read(STDIN_FILENO, &kbd_pressed_key_, 1);
            {
                std::lock_guard<std::mutex> lock(mutex_key_press_);
                if (kbd_pressed_key_ == 'S') state_ = STATE_STEPPING;
                else if (kbd_pressed_key_ == 'R') state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == 'P' && state_ != STATE_PAUSED) state_ = STATE_PAUSED;
                else if (kbd_pressed_key_ == 'P' && state_ == STATE_PAUSED) state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == KEY_ESC) state_ = STATE_STOPPED;
                else if (kbd_pressed_key_ == '+' && freq_ < 2000) freq_ += 50;
                else if (kbd_pressed_key_ == '-' && freq_ > 50) freq_ -= 50;
                else if (kbd_pressed_key_ == 'Q') use_quirks_ = !use_quirks_;
                if (keyboard2keypad_.find(kbd_pressed_key_) != keyboard2keypad_.end())
                    pressed_keys_[keyboard2keypad_[kbd_pressed_key_]] = true;
            }
        }
    }
}


uint8_t Chip8::WaitForKey() {
    char ch;
    do {
        ch = getchar(); 
    } while (keyboard2keypad_.find(ch) == keyboard2keypad_.end());
    return keyboard2keypad_[ch];
}

inline void Chip8::Cls() {
    frame_buffer_.fill(0);
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
}

void Chip8::RenderFrame() {
    TPRINT_GOTO_TOPLEFT();
    std::string horizontal = "";
    for (int i = 0; i < COLS; i++)
        horizontal += u8"\u2500";
    const std::string border_up = u8"\u250C" + horizontal + u8"\u2510" + "\n";
    const std::string border_down = u8"\u2514" + horizontal + u8"\u2518" + "\n";
    std::string pixels = border_up;
    for (size_t row = 0; row < ROWS; ++row) {
        static std::array<char, COLS> line_buffer {};
        for (size_t col = 0; col < COLS; ++col) {
            const size_t index = row * COLS + col;
            frame_buffer_[index] != 0 ? line_buffer[col] = 24 : line_buffer[col] = ' ';
        }
        std::string frame_row(line_buffer.begin(), line_buffer.end());
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
    std::cout << pixels << std::endl <<  std::flush;
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

void Chip8::UpdateTimers() {
    while (run_timers_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        if (delay_timer_ > 0) --delay_timer_;
        if (sound_timer_ > 0) --sound_timer_;
    }
}
