# 1. About Chip-8
```
   ________    _             ____ 
  / ____/ /_  (_)___        ( __ )
 / /   / __ \/ / __ \______/ __  |
/ /___/ / / / / /_/ /_____/ /_/ / 
\____/_/ /_/_/ .___/      \____/  
            /_/                   
```

### 1.1 What is Chip-8?
Chip-8 is not an actual hardware. It's a a virtual machine (like Java) that was implemented by a number of computers in the 70s and calculators in the 80s. It was designed for people to program really early home microcomputers. Instead of using actual microprocessor opcodes, it was always designed to be a virtual language and be interpreted at runtime ([reference](http://www.emulator101.com/introduction-to-chip-8.html)).

### 1.2 Its architecture
Chip-8 operates on 4 kB of RAM memory and certain partitions of it are reserved for the interpreter, the loaded program (rom), and some hardcoded font sprites. It has 16 general-purpose 8-bit registers prfixed by `V`, one special 16-bit register called `I` which points to memory locations, and two special 8-bit sound and timer registers which tick at 60 Hz.  

It also possesses a 16-bit stack pointer (`SP`) which points to the stack and a 16-bit program counter (`PC`), which points to the current instruction.  

It's able to play  a beeping sound (1 frequency) as long as the sound timer is non zero, however I couldn't find anything about its specific frequency or duration.  

It can render graphics on a 64x32 monochrome display.  

The neat thing about this machine is that each instruction always consists of 2 consecutive bytes, which makes it easy to process them. In total, Chip-8 language consists of 36 instructions, however extensions such as Super Chip-8 possess more. The instructions (opcodes) also interact with the keyboard and display. [Cowgod](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0) has written the holy grail of Chip-8 documentation.


### 1.3 The keypad
The keypad consists of 16 keys, numbered in hex from `0` to `9` to `a` to `f`. When emulated, these keys are mapped to a modern keyboard's (such as qwerty) keys.

# 2. This project

### 2.1 Prerequisites
The only needed library is SDL2 for the display. On Debian-based systems, you can install it with:
```
sudo apt-get install libsdl2-dev
```

### 2.2 How to compile and run
It can be compiled on \*nix systems with make (see Makefile):
```
cd chip-8
make
```
The generated executable is called `chip8`, which takes a rom file (`roms` folder) as argument. For example, to play Brix:
```
./chip8 roms/Brix.ch8
```
To clean the generated object files and executable:
```
make clean
```

### 2.3 Emulation configs - the .ini file
If you want to change an emulation setting, such as how fast a rom shall run, muting the sound etc., you can edit the `chip8.ini` file, recompile, and run. The settings of the .ini file are documented in it.

### 2.4 Unit tests
Unit tests can be added monolithically in file `tests/tests.cpp` and are built in conjunction with the [Catch2](https://github.com/catchorg/Catch2) library. To build and run them, do:
```
make test
```

### 2.5 Features implemented 
- [x] Machine architecture, display, and keypad.
- [x] Emulate roms at a fixed speed of certain opcodes per sec - 400 in my case.
- [x] Sound effects and frequency selection - credits to [vareille's toot library](https://github.com/vareille/toot).
- [x] Pause rom - `F1` key
- [x] Exit - `Esc` key or close window
- [x] Configure emulation settings via .ini file
- [x] Mute option (in .ini file)
- [ ] Reload rom - TODO
- [ ] Select background and foreground colours on the display - TODO

### 2.6 The keypad

The Chip-8 to modern keyboard mapping was defined like this:
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
If you wish to change the mapping, you have to edit the following line and recompile:
https://github.com/0xLeo/chip-8/blob/master/include/keyboard.hpp#L15

Apart from the keypad, the following keys are implemented to facilitate the UI:
* `F1` - pause execution
* `Esc` or close window - exit


### 2.7 Available roms
Some classical game roms, such as Pong, Tetris and Brix, are included in the `roms` folder. These are public domain and originally found in [dmatlack's repository](https://github.com/dmatlack/chip8), so credits to him. The binary file of each rom ends in `.ch8`. Also, a corresponding README manual for each one is provided (again, originally uploaded by dmatlack) as a .txt file. The README explains the keys used.


### 2.8 Demos

![](https://raw.githubusercontent.com/0xLeo/chip-8/master/pics/pong.gif)  |  ![](https://raw.githubusercontent.com/0xLeo/chip-8/master/pics/brix.gif)
:-------------------------:|:-------------------------:
![](https://raw.githubusercontent.com/0xLeo/chip-8/master/pics/coin.gif)  |  ![](https://raw.githubusercontent.com/0xLeo/chip-8/master/pics/tetris.gif)

# 3.Credits
* The legendary [bisqwit](https://www.youtube.com/watch?v=rpLoS7B6T94) on youtube for the inspiration
* [cowgod](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#0.0) for the comprehensive documentation
* Tobias V. Langhoff for his [guidelines and tips](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
* [dmatlack](https://github.com/dmatlack/chip8) for his publicly available roms
* [vareille](https://github.com/vareille/) for his [C/C++ sound library](https://github.com/vareille/toot).
* the [emudev subreddit](https://www.reddit.com/r/EmuDev/) for having so many answered questions regarding Chip-8
* [Catch2](https://github.com/catchorg/Catch2) library for unit testing
