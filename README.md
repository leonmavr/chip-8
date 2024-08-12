```
    ▒▒▒▒▒▒ ▒▒▒▒▒         ▒▒▒▒▒ ▒▒▒▒▒▒▒▒ ▒▒▒▒▒▒      ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒

  ▒▒▒▒▒▒    ▒▒▒▒         ▒▒▒▒  ▒▒▒▒▒▒▒▒ ▒▒▒▒▒▒▒▒▒   ▒▒▒▒▒       ▒▒▒▒▒
                   
▒▒▒▒▒▒       ▒▒▒         ▒▒▒     ▒▒▒▒   ▒▒▒▒▒▒▒▒▒▒▒ ▒▒▒▒▒       ▒▒▒▒▒
          
▒▒▒▒▒▒       ▒▒▒ ▒▒▒▒▒▒▒ ▒▒▒     ▒▒▒▒   ▒▒▒▒▒▒▒▒▒   ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
                                                       
▒▒▒▒▒▒       ▒▒▒ ▒▒▒▒▒▒▒ ▒▒▒     ▒▒▒▒   ▒▒▒▒        ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒

▒▒▒▒▒        ▒▒▒         ▒▒▒     ▒▒▒▒   ▒▒▒▒        ▒▒▒▒▒       ▒▒▒▒▒

  ▒▒▒▒▒▒    ▒▒▒▒         ▒▒▒▒  ▒▒▒▒▒▒▒▒ ▒▒▒▒        ▒▒▒▒▒       ▒▒▒▒▒

    ▒▒▒▒▒▒ ▒▒▒▒▒         ▒▒▒▒▒ ▒▒▒▒▒▒▒▒ ▒▒▒▒        ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
```
# 1. About

I wrote a Chip8 virtual machine. Chip8 was implemented by a number of 
[computers](http://www.hobbylabs.org/telmac.htm) in the 70s and HP 
[calculators](https://www.hpcalc.org/hp48/docs/faq/48faq-8.html) in the 80s.
Instead of using actual microprocessor opcodes, it was always designed to be a 
virtual language and be interpreted at 
[runtime](http://www.emulator101.com/introduction-to-chip-8.html).

<img src="https://github.com/leonmavr/chip-8/blob/master/assets/demo_grid.gif" alt="Demo gif" height="300px">

(A high quality video version of the gif is found
[here](https://github.com/leonmavr/chip-8/blob/master/assets/demo_grid.mp4).)

# 2. Usage

This project has no third-party dependencies and renders on the terminal.  
To compile:
```bash
make
```
To run:
```bash
./play path/to/rom.ch8
```
Compile and run the sanity tests (mostly for CI):
```bash
make test
```
To clean all object and executable files:
```bash
make clean
```

# 3. Features

- [x] CPU and renderer.
- [x] UI including controls and debugger view including pause, exit, and stepping key.
- [x] .cfg file each ROM; this lists the controls and sets the emulation
      frequency for each ROM. Found at `roms/*.cfg`.
- [x] Configurable keys defined at [keypad.hpp](https://github.com/leonmavr/chip-8/blob/master/include/keypad.hpp).
- [x] CI.
- [ ] Sound; probably never going to implement this.

# 4. Architecture

**TODO**

Chip-8 operates on 4 kB of RAM memory and certain partitions of it are reserved for the interpreter, the loaded program (rom), and some hardcoded font sprites. It has 16 general-purpose 8-bit registers prfixed by `V`, one special 16-bit register called `I` which points to memory locations, and two special 8-bit sound and timer registers which tick at 60 Hz.  

It also possesses a 16-bit stack pointer (`SP`) which points to the stack and a 16-bit program counter (`PC`), which points to the current instruction.  

It's able to play  a beeping sound (1 frequency) as long as the sound timer is non zero, however I couldn't find anything about its specific frequency or duration.  

It can render graphics on a 64x32 monochrome display.  

The neat thing about this machine is that each instruction always consists of 2 consecutive bytes, which makes it easy to process them. In total, Chip-8 language consists of 36 instructions, however extensions such as Super Chip-8 possess more. The instructions (opcodes) also interact with the keyboard and display. [Cowgod](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0) has written the holy grail of Chip-8 documentation.

The Chip-8 to modern keyboard mapping was defined by default like this:
```
+---+---+---+---+       +---+---+---+---+
| 0 | 1 | 2 | 3 |       | 1 | 2 | 3 | 4 |
+---+---+---+---+       +---+---+---+---+
| 4 | 5 | 6 | 7 |       | q | w | e | r |
+---+---+---+---+  -->  +---+---+---+---+
| 8 | 9 | a | b |       | a | s | d | f |
+---+---+---+---+       +---+---+---+---+
| c | d | e | f |       | z | x | c | v |
+---+---+---+---+       +---+---+---+---+
```
