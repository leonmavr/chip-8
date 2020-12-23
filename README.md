# About Chip-8
```
   ________    _             ____ 
  / ____/ /_  (_)___        ( __ )
 / /   / __ \/ / __ \______/ __  |
/ /___/ / / / / /_/ /_____/ /_/ / 
\____/_/ /_/_/ .___/      \____/  
            /_/                   
```

### What is Chip-8?
Chip-8 is not an actual hardware. It's a a virtual machine (like Java) that was implemented by a number of computers in the 70s and calculators in the 80s. It was designed for people to program really early home microcomputers. Instead of using actual microprocessor opcodes, it was always designed to be a virtual language and be interpreted at runtime ([reference](http://www.emulator101.com/introduction-to-chip-8.html)).

### Its architecture
Chip-8 operates on 4 kB of RAM memory and certain partitions of it are reserved for the interpreter, the loaded program (rom), and some hardcoded font sprites. It has 16 general-purpose 8-bit registers prfixed by `V`, one special 16-bit register called `I` which points to memory locations, and two special 8-bit sound and timer registers which tick at 60 Hz.  

It also possesses a 16-bit stack pointer (`SP`) which points to the stack and a 16-bit program counter (`PC`), which points to the current instruction.  

It's able to play  a beeping sound (1 frequency) as long as the sound timer is non zero, however I couldn't find anything about its specific frequency or duration.  

It can render graphics on a 64x32 monochrome display.  

The neat thing about this machine is that each instruction always consists of 2 consecutive bytes, which makes it easy to process them. In total, Chip-8 language consists of 36 instructions, however extensions such as Super Chip-8 possess more. The instructions (opcodes) also interact with the keyboard and display. [Cowgod](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0) has documented in detail the architecture.


### The keypad
The keypad consists of 16 keys, numbered in hex from `0` to `9` to `a` to `f`. When emulated, these keys are mapped to a modern keyboard's (such as qwerty) keys.

# This project

### Prerequisites
The only needed library is SDL2 for the display. On Debian-based systems, you can install it with:
```
sudo apt-get install libsdl2-dev
```

### How to compile and run
It can be compiled on \*nix systems with make (see Makefile):
```
cd chip-8
make
```
The generated executable is called `chip8`, which takes a rom file (`rom` folder) as argument. For example, to play Brix:
```
./chip8 roms/Brix.ch8
```
To clean the generated object files and executable:
```
make clean
```

### Features implemented 
- [x] Machine architecture, display, and keypad.
- [x] Emulate roms at a fixed speed of certain opcodes per sec - 400 in my case.
- [x] Sound - credits to [vareille's toot library](https://github.com/vareille/toot).
- [x] Pause rom - `F1` key
- [x] Exit - `Esc` key or close window
- [ ] Reload rom - TODO
- [ ] Select backgrond and foregound colours on the display - TODO

### The keypad

Apart from the keypad, the following keys are implemented to facilitate the UI:
* `F1` - pause execution
* `Esc` or close window - exit


### Available roms
Some classical game roms, such as Pong, Tetris and Brix, are included in the `roms` folder. These are public domain and originally found in [dmatlack's repository](https://github.com/dmatlack/chip8), so credits to him. The binary file of each rom ends in `.ch8`. Also, a corresponding README manual for each one is provided (again, originally uploaded by dmatlack) as a .txt file. The README explains the keys used.


### Demos

### Credits

### References
http://www.emulator101.com/introduction-to-chip-8.html
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0
https://github.com/dmatlack/chip8
