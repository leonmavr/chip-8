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
    regs_{},
    stack_{},
    SP_(0x0),
    PC_(ROM_OFFSET),
    I_(0x000),
    pixels_{},
    delay_timer_(0x00),
    sound_timer_(0x00),
    run_timers_(true),
    stop_key_thread_(false),
    state_(STATE_RUNNING),
    freq_(250),
    kbd_pressed_key_('\0'),
    timer_thread_(std::thread(&Chip8::UpdateTimers, this)),
    key_thread_(std::thread(&Chip8::PressKey, this)),
    cfg_parser_(nullptr)
{
    // write sprites to memory
    const std::vector<uint8_t> font_sprites = {
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
        key_states_[i] = is_pressed;
    SetNonBlockingInput();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    TPRINT_HIDE_CURSOR();
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
    freq_ = cfg_parser_->GetFrequency();
}

inline uint8_t Chip8::Rand() const {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<uint8_t> dist(0, 255);
    return dist(rng);
}

inline uint16_t Chip8::Fetch() const {
    uint16_t instr = ram_[PC_] << 8;
    instr |= ram_[PC_ + 1];
    return instr;
}


inline opcode_t Chip8::Decode(uint16_t instr) const {
    // Extract bit-fields from the opcode
    // see http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.0
    // for bitfield explanation
    opcode_t decoded;
    decoded.prefix = (instr >> 12) & 0x000f;
    decoded.n      = instr         & 0x000f;
    decoded.x      = (instr >> 8)  & 0x000f;
    decoded.y      = (instr >> 4)  & 0x000f;
    decoded.nn     = instr         & 0x00ff;
    decoded.nnn    = instr         & 0x0fff;
    return decoded;
}


void Chip8::Exec(opcode_t opc) {
    const auto x = opc.x;
    const auto y = opc.y;
    const auto n = opc.n;
    const auto nn = opc.nn;
    const auto nnn = opc.nnn;
    const auto prefix = opc.prefix;

    auto& Vx = regs_[x];
    auto& Vy = regs_[y];
    /* detects overflow, e.g. in additions */
    auto& Vf = regs_[0xf];
    auto& I = I_;
    auto& PC = PC_;

#define EXEC_INSTRUCTION \
    /* assembly, condition, instruction(s) */ \
    X("CLS"        , prefix == 0x0 && nnn == 0x0e0 , Cls();) \
    X("RET"        , prefix == 0x0 && nnn == 0x0ee , PC = stack_[--SP_];) \
    X("JP nnn"     , prefix == 0x1                 , PC = nnn - 2;) \
    X("CALL nnn"   , prefix == 0x2                 , stack_[SP_++] = PC; PC = nnn - 2;) \
    X("SE Vx nn"   , prefix == 0x3 && nn == Vx     , PC += 2;) \
    X("SNE Vx nn"  , prefix == 0x4 && nn != Vx     , PC += 2;) \
    X("SE Vx Vy"   , prefix == 0x5 && Vx == Vy     , PC += 2;) \
    X("LD Vx nn"   , prefix == 0x6                 , Vx = nn;) \
    X("ADD Vx nn"  , prefix == 0x7                 , Vx += nn;) \
    X("LD Vx Vy"   , prefix == 0x8 && n == 0x0     , Vx = Vy;) \
    X("OR Vx Vy"   , prefix == 0x8 && n == 0x1     , Vx |= Vy;) \
    X("AND Vx Vy"  , prefix == 0x8 && n == 0x2     , Vx &= Vy;) \
    X("XOR Vx Vy"  , prefix == 0x8 && n == 0x3     , Vx ^= Vy;) \
    X("ADD Vx Vy"  , prefix == 0x8 && n == 0x4     , Vf = ((Vx & 0x80) >> 7) & ((Vy & 0x80) >> 7); Vx += Vy;) \
    X("SUB Vx Vy"  , prefix == 0x8 && n == 0x5     , Vf = (Vx > Vy) ? 1 : 0; Vx -= Vy;) \
    X("SHR Vx Vy"  , prefix == 0x8 && n == 0x6     , Vf = Vx & 1; Vx >>= 1;) \
    X("SUBN Vx Vy" , prefix == 0x8 && n == 0x7     , Vf = (Vy > Vx) ? 1 : 0; Vx = Vy - Vx;) \
    X("SHL Vx Vy"  , prefix == 0x8 && n == 0xe     , Vf = (Vx >> 7) & 0x1; Vx <<= 1;) \
    X("SNE Vx Vy"  , prefix == 0x9 && Vx != Vy     , PC += 2;) \
    X("LD I nnn"   , prefix == 0xa                 , I = nnn;) \
    X("JP V0 nnn"  , prefix == 0xb                 , PC = nnn + regs_[0] - 2;) \
    X("RND Vx nn"  , prefix == 0xc                 , Vx = Rand() & nn;) \
    X("DRW Vx Vy n", prefix == 0xd,                     \
        do {                                            \
            Vf = 0;                                     \
            for (uint8_t row = 0; row < n; row++) {     \
                uint8_t sprite = ram_[I + row];         \
                for (uint8_t col = 0; col < 8; col++) { \
                    if ((sprite & 0x80) != 0) {         \
                        size_t x = (Vx + col) % COLS;   \
                        size_t y = (Vy + row) % ROWS;   \
                        size_t index = y * COLS + x;    \
                        if (pixels_[index] == 1)        \
                            Vf = 1;                     \
                        pixels_[index] ^= 1;            \
                    }                                   \
                    sprite <<= 1;                       \
                }                                       \
            }                                           \
        } while(0); )                                   \
X("SKP Vx"         , prefix == 0xe && nn == 0x9e   , if ( key_states_[Vx & 0xF]) PC += 2;) \
X("SKNP Vx"        , prefix == 0xe && nn == 0xa1   , if (!key_states_[Vx & 0xF]) PC += 2;) \
X("LD Vx DT"       , prefix == 0xf && nn == 0x07   , Vx = delay_timer_;) \
X("LD Vx k"        , prefix == 0xf && nn == 0x0a   , Vx = WaitForKey();) \
X("LD DT Vx"       , prefix == 0xf && nn == 0x15   , delay_timer_ = Vx;) \
X("LD ST Vx"       , prefix == 0xf && nn == 0x18   , sound_timer_ = Vx;) \
X("ADD I Vx"       , prefix == 0xf && nn == 0x1e   , Vf = (I + Vx > 0xfff) ? 1 : 0; I += Vx;) \
X("LD F Vx"        , prefix == 0xf && nn == 0x29   , I = Vx * 5;) \
X("LD B Vx"        , prefix == 0xf && nn == 0x33   , ram_[(I + 0) & 0xfff] = (Vx % 1000) / 100; \
                                                     ram_[(I + 1) & 0xfff] = (Vx % 100) / 10; \
                                                     ram_[(I + 2) & 0xfff] = Vx % 10;) \
X("LD [I] Vx"      , prefix == 0xf && nn == 0x55    , for (unsigned xx = 0; xx <= x; xx++) \
                                                          ram_[I++ & 0xfff] = regs_[xx];) \
X("LD Vx [I]"      , prefix == 0xf && nn == 0x65    , for (unsigned xx = 0; xx <= x; xx++) \
                                                          regs_[xx] = ram_[I++ & 0xfff];) 

#define X(assembly, condition, instructions) if (condition) { instructions; }
EXEC_INSTRUCTION
#undef X
#undef EXEC_INSTRUCTION
}

void Chip8::Run() {
    auto t_keyboard_start = std::chrono::high_resolution_clock::now();
    // get current time in ms
    auto now_ms = []() -> unsigned {
        auto now = std::chrono::system_clock::now();
        auto since_epoch = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count();
    };
    // sleep for some ms
    auto sleep_ms = [](unsigned ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    };
    // t0 and t1 enforce the loop to cycle at frequency `freq_`
    unsigned t0 = now_ms(), t1 = now_ms();
    // Helps throttle the instructions run per second to `freq_` instructions per second.
    // It checks how many instructions are run every 0.1 sec and if it's more than `freq_/10`, it
    // stalls the fetch-decode-execute loop until the instructions catch up.
    static unsigned instr_per_50ms = 0;

    while (true) {
        if (state_ == STATE_PAUSED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            RenderAll();
            continue;
        } else if (state_ == STATE_STEPPING) {
            kbd_pressed_key_ = '\0';
            while (kbd_pressed_key_ != 'S' && kbd_pressed_key_ != 'R' && kbd_pressed_key_ != 'P') {
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
        Chip8::Exec(opc);
        RenderAll();

        auto t_keyboard_end = std::chrono::high_resolution_clock::now();
        auto dt_keyboard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_keyboard_end - t_keyboard_start);


        if (dt_keyboard_ms.count() >= 100) {
            std::lock_guard<std::mutex> lock(key_states_mutex_);
            for (auto& pair : key_states_)
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
    }
}


Chip8::~Chip8 () {
    run_timers_ = false;
    if (timer_thread_.joinable())
        timer_thread_.join();
    stop_key_thread_ = true;
    if (key_thread_.joinable())
        key_thread_.join();
    ResetBlockingInput();
};

void Chip8::PressKey() {
    while (!stop_key_thread_) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 40000;

        int success = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        if (success > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            read(STDIN_FILENO, &kbd_pressed_key_, 1);
            {
                std::lock_guard<std::mutex> lock(key_states_mutex_);
                if (kbd_pressed_key_ == 'S') state_ = STATE_STEPPING;
                else if (kbd_pressed_key_ == 'R') state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == 'P' && state_ != STATE_PAUSED) state_ = STATE_PAUSED;
                else if (kbd_pressed_key_ == 'P' && state_ == STATE_PAUSED) state_ = STATE_RUNNING;
                else if (kbd_pressed_key_ == KEY_ESC) state_ = STATE_STOPPED;
                else if (kbd_pressed_key_ == '+' && freq_ < 2000) freq_ += 50;
                else if (kbd_pressed_key_ == '-' && freq_ > 50) freq_ -= 50;
                if (keyboard2keypad_.find(kbd_pressed_key_) != keyboard2keypad_.end())
                    key_states_[keyboard2keypad_[kbd_pressed_key_]] = true;
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

void Chip8::Cls() {
    pixels_.fill(0);
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
}

void Chip8::RenderAll() {
    // display pixels
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    std::string border_up_down = "+" + std::string(64, '-') + "+\n";
    std::string pixels = border_up_down;
    for (size_t row = 0; row < ROWS; ++row) {
        std::array<uint8_t, COLS> line {};
        for (size_t col = 0; col < COLS; ++col) {
            size_t index = row * COLS + col;
            pixels_[index] != 0 ? line[col] = 24 : line[col] = ' ';
        }
        std::string pixel_row(line.begin(), line.end());
        const std::string border_left_right = "|";
        pixels += border_left_right + pixel_row + border_left_right;
        pixels += "\n";
    }
    pixels += border_up_down;
    // debugger and keyboard controls
    Frontend::WriteRegs(pixels, regs_);
    Frontend::WriteI(pixels, I_);
    Frontend::WritePC(pixels, PC_);
    Frontend::WriteSP(pixels, SP_);
    Frontend::WriteStack(pixels, stack_);
    int line = 10;
    Frontend::WriteRight(pixels, line++, "[P]ause/Unpause [S]tep [R]un [Esc]ape\n");
    Frontend::WriteRight(pixels, line++, "[-] " + std::to_string(freq_) + " Hz [+]\n");
    for (const auto& key_descr: cfg_parser_->GetKeyMap()) {
        std::string key = key_descr.first;
        std::string descr = key_descr.second;
        Frontend::WriteRight(pixels, line++, "[" + key + "] " + descr + "\n");
    }
    std::cout << pixels << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(1600));
}

void Chip8::UpdateTimers() {
    while (run_timers_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        if (delay_timer_ > 0) --delay_timer_;
        if (sound_timer_ > 0) --sound_timer_;
    }
}
