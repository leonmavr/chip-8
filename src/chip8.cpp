#include <random>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <termios.h>
#include <thread>
#include <thread>
#include <termios.h>
#include <fcntl.h>
#include "chip8.hpp" 
#include "toot.h" 
#include "logger.hpp" 
#include "term.h" 


// helps throttle the instructions run per second to `m_instrPerSec` by stalling every 0.1 sec if necessary
static unsigned execInsrPerSec = 0;


Chip8::Chip8(std::string fnameIni):
    m_instrPerSec(1000),
    m_mute(false),
    m_maxIter(-1),
    pixels_{0},
    pixels_prev_{0},
    delay_timer_(0x00),
    sound_timer_(0x00),
    run_timers_(true)
{
    // init display
    timer_thread_ = std::thread(&Chip8::UpdateTimers, this);
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    TPRINT_HIDE_CURSOR();
    Chip8::init();
}


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


uint16_t Chip8::fetch() {
    // TODO: remove assert when everything is ok
    //assert(m_PC < 0x1000);
    uint16_t instr = m_mem[m_PC] << 8;
    instr |= m_mem[m_PC + 1];
    return instr;
}


opcode_t Chip8::decode(uint16_t instr) {
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


void Chip8::exec(opcode_t opc) {
    const auto x = opc.x;
    const auto y = opc.y;
    const auto n = opc.n;
    const auto nn = opc.nn;
    const auto nnn = opc.nnn;
    const auto prefix = opc.prefix;

    auto& Vx = m_V[x];
    auto& Vy = m_V[y];
    /* detects overflow, e.g. in additions */
    auto& Vf = m_V[0xf];
    auto& I = m_I;
    auto& PC = m_PC;

#define EXEC_INSTRUCTION \
    /* assembly, condition, instruction(s) */ \
    X("CLS", prefix == 0x0 && nnn == 0x0e0, cls();) \
    X("RET", prefix == 0x0 && nnn == 0x0ee, PC = m_stack[--m_SP % 12];) \
    X("JP nnn", prefix == 0x1, PC = nnn - 2;) \
    X("CALL nnn", prefix == 0x2, m_stack[m_SP++ % 12] = PC; PC = nnn - 2;) \
    X("SE Vx nn", prefix == 0x3 && nn == Vx, PC += 2;) \
    X("SNE Vx nn", prefix == 0x4 && nn != Vx, PC += 2;) \
    X("SE Vx Vy", prefix == 0x5 && Vx == Vy, PC += 2;) \
    X("LD Vx nn", prefix == 0x6, Vx = nn;) \
    X("ADD Vx nn", prefix == 0x7, Vx += nn;) \
    X("LD Vx Vy", prefix == 0x8 && n == 0x0, Vx = Vy;) \
    X("OR Vx Vy", prefix == 0x8 && n == 0x1, Vx |= Vy;) \
    X("AND Vx Vy", prefix == 0x8 && n == 0x2, Vx &= Vy;) \
    X("XOR Vx Vy", prefix == 0x8 && n == 0x3, Vx ^= Vy;) \
    X("ADD Vx Vy", prefix == 0x8 && n == 0x4, Vf = (Vx & 0x80) & (Vy & 0x80); Vx += Vy;) \
    X("SUB Vx Vy", prefix == 0x8 && n == 0x5, Vf = (Vx > Vy) ? 1 : 0; Vx -= Vy;) \
    X("SHR Vx Vy", prefix == 0x8 && n == 0x6, Vf = Vx & 1; Vx = Vy >>= 1;) \
    X("SUBN Vx Vy", prefix == 0x8 && n == 0x7, Vf = (Vy > Vx) ? 1 : 0; Vx = Vy - Vx;) \
    X("SHL Vx Vy", prefix == 0x8 && n == 0xe, Vf = (Vy >> 7) & 0x1; Vx = Vy << 1;) \
    X("SNE Vx Vy", prefix == 0x9 && Vx != Vy, PC += 2;) \
    X("LD I nnn", prefix == 0xa, I = nnn;) \
    X("JP V0 nnn", prefix == 0xb, PC = nnn + m_V[0] - 2;) \
    X("RND Vx nn", prefix == 0xc, Vx = rand() & nn;) \
    X("DRW Vx Vy n", prefix == 0xd, \
        do { \
            Vf = 0;\
            for (uint8_t row = 0; row < n; row++) {\
                uint8_t sprite = m_mem[I + row];\
            for (uint8_t col = 0; col < 8; col++) {\
                if ((sprite & 0x80) != 0) {\
                    size_t x = (Vx + col) % COLS;\
                    size_t y = (Vy + row) % ROWS;\
                    size_t index = y * COLS + x;\
                    if (pixels_[index] == 1)\
                        Vf = 1;\
                    pixels_[index] ^= 1;\
                }\
                sprite <<= 1;\
            }\
        }\
    } while(0); ) \
X("SKP Vx", prefix == 0xe && nn == 0x9e, if (key_states_[Vx & 0xF]) PC += 2;) \
X("SKNP Vx", prefix == 0xe && nn == 0xa1, if (!key_states_[Vx & 0xF]) PC += 2;) \
X("LD Vx DT", prefix == 0xf && nn == 0x07, Vx = delay_timer_;) \
X("LD Vx k", prefix == 0xf && nn == 0x0a, Vx = WaitForKey();) \
X("LD DT Vx", prefix == 0xf && nn == 0x15, delay_timer_ = Vx;) \
X("LD ST Vx", prefix == 0xf && nn == 0x18, sound_timer_ = Vx;) \
X("ADD I Vx", prefix == 0xf && nn == 0x1e, Vf = (I + Vx > 0xfff) ? 1 : 0; I += Vx;) \
X("LD F Vx", prefix == 0xf && nn == 0x29, I = Vx * 5;) \
X("LD B Vx", prefix == 0xf && nn == 0x33, \
    m_mem[(I + 0) & 0xfff] = (Vx % 1000) / 100; \
    m_mem[(I + 1) & 0xfff] = (Vx % 100) / 10; \
    m_mem[(I + 2) & 0xfff] = Vx % 10;) \
X("LD [I] Vx", prefix == 0xf && nn == 0x55, \
    for (unsigned xx = 0; xx <= x; xx++) \
        m_mem[I++ & 0xfff] = m_V[xx];) \
X("LD Vx [I]", prefix == 0xf && nn == 0x65, \
    for (unsigned xx = 0; xx <= x; xx++) \
        m_V[xx] = m_mem[I++ & 0xfff];)

#define X(assembly, condition, instructions) \
if (condition) \
{ instructions; }
EXEC_INSTRUCTION

PC += 2;

#undef X
#undef EXEC_INSTRUCTION

// decrement every 60 hz and play sound if necessary
//if (execInsrPerSec % 2 == 0) {
#if 0
    if (m_delayTimer > 0) 
        m_delayTimer--;
    if (m_soundTimer > 0)
        m_soundTimer--;
        // TODO: beep if timer 0:
#endif
//}
//execInsrPerSec++;
}


void Chip8::run(unsigned startingOffset) {
std::chrono::high_resolution_clock::time_point t_start, t_end;
int t_deltaUs;

m_PC = startingOffset;
// the trick to stop the loop is when 2 consecutive bytes of free space (0xff) are encountered
auto t_keyboard_start = std::chrono::high_resolution_clock::now();
auto t_throttle_start = std::chrono::high_resolution_clock::now();

while (1) {

    // fetch-decode-exec defines the operation of Chip8
    uint16_t instr = fetch();
    opcode_t opc = decode(instr);

    
    PressKey();
    auto t_keyboard_end = std::chrono::high_resolution_clock::now();
    auto dt_keyboard = std::chrono::duration_cast<std::chrono::milliseconds>(t_keyboard_end - t_keyboard_start);

    Chip8::exec(opc);
    renderAll();
    if (dt_keyboard.count() >= 50) {
        for (auto& pair: key_states_)
            pair.second = false;
        t_keyboard_start = t_keyboard_end;
    }
    t_end = std::chrono::high_resolution_clock::now();
    if ((execInsrPerSec % every_100_ms) == 0) {
        auto t_throttle_end = std::chrono::high_resolution_clock::now();
        auto dt_throttle = std::chrono::duration_cast<std::chrono::milliseconds>(t_throttle_end - t_throttle_start);
        if (dt_throttle.count() < 100) {
            //std::cout << "waittt\n";
            std::this_thread::sleep_for(std::chrono::microseconds(100000 - 1000*dt_throttle.count()));
            // wait till we reach ... ms
            t_throttle_start = std::chrono::high_resolution_clock::now();
            //toot(400,100);
        }
            
    }
    execInsrPerSec++;

#if 0
    if (execInsrPerSec/2 >= m_instrPerSec/2) {
            t_end = std::chrono::high_resolution_clock::now();
            t_deltaUs = (t_end - t_start)/std::chrono::milliseconds(1)*1000;
            std::this_thread::sleep_for(std::chrono::microseconds(
                        static_cast<int>(10000 > t_deltaUs) * (10000 - t_deltaUs)
                        ));
            execInsrPerSec = 0;
        }
#endif
    }
}
static struct termios orig_termios;

static void SetNonBlockingInput() {
    // Save the original terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios tty = orig_termios;
    // Disable canonical mode and echo
    tty.c_lflag &= ~(ICANON | ECHO);
    // Set new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
    // Set non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}


static void ResetBlockingInput() {
    // Restore the original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    // Reset blocking mode
    fcntl(STDIN_FILENO, F_SETFL, 0);
}


void Chip8::init() {
    // 1. Initialise special registers and memory
    m_SP = 0x0;
    m_PC = 0x200;
    m_I = 0x0;
    for (auto& m: m_mem)
        m = 0x0;
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

    SetNonBlockingInput();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
}

void Chip8::PressKey() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    char ch;
    int success = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    if (success > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        read(STDIN_FILENO, &ch, 1);
        key_states_[keyboard2keypad_[ch]] = true;
    }
}

uint8_t Chip8::WaitForKey() {
    char ch;
    do {
        ch = getchar(); 
    } while (keyboard2keypad_.find(ch) == keyboard2keypad_.end());
    return keyboard2keypad_[ch];
}

void Chip8::cls() {
    TPRINT_HIDE_CURSOR();
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    pixels_.fill(0);
    pixels_prev_.fill(0);
}

void Chip8::renderAll() {
    TPRINT_GOTO_TOPLEFT();
    TPRINT_CLEAR();
    std::string border_up_down = "+" + std::string(64, '-') + "+\n";
    std::string pixels = border_up_down;
    for (size_t row = 0; row < ROWS; ++row) {
        std::array<uint8_t, COLS> line {};
        for (size_t col = 0; col < COLS; ++col) {
            size_t index = row * COLS + col;
            if (pixels_[index] != 0) {
                line[col] = 24;
            } else {
                line[col] = 32;
            }
        }
        std::string pixel_row(line.begin(), line.end());
        const std::string border_left_right = "|";
        pixels += border_left_right + pixel_row + border_left_right;
        pixels += "\n";
    }
    pixels += border_up_down;
    std::cout << pixels << std::endl;
    std::this_thread::sleep_for(std::chrono::microseconds(500));
}

void Chip8::UpdateTimers() {
    while (run_timers_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        if (delay_timer_ > 0) --delay_timer_;
        if (sound_timer_ > 0) --sound_timer_;
    }
}
