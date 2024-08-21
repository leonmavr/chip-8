```
    ▒▒▒▒▒▒ ▒▒▒▒▒       ▒▒▒▒▒ ▒▒▒▒▒▒▒▒ ▒▒▒▒▒▒▒▒▒    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
  ▒▒▒▒▒▒    ▒▒▒▒       ▒▒▒▒  ▒▒▒▒▒▒▒▒ ▒▒▒▒▒▒▒▒▒▒▒  ▒▒▒▒       ▒▒▒▒
▒▒▒▒▒▒       ▒▒▒       ▒▒▒     ▒▒▒▒   ▒▒▒   ▒▒▒▒▒▒ ▒▒▒▒       ▒▒▒▒
▒▒▒▒▒▒       ▒▒▒▒▒▒▒▒▒▒▒▒▒     ▒▒▒▒   ▒▒▒  ▒▒▒▒▒▒  ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒       ▒▒▒▒▒▒▒▒▒▒▒▒▒     ▒▒▒▒   ▒▒▒▒▒▒▒▒▒    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒       ▒▒▒       ▒▒▒     ▒▒▒▒   ▒▒▒▒▒        ▒▒▒▒       ▒▒▒▒
  ▒▒▒▒▒▒    ▒▒▒▒       ▒▒▒▒  ▒▒▒▒▒▒▒▒ ▒▒▒▒▒        ▒▒▒▒       ▒▒▒▒
    ▒▒▒▒▒▒ ▒▒▒▒▒       ▒▒▒▒▒ ▒▒▒▒▒▒▒▒ ▒▒▒▒▒        ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
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

### 2.1 Compilation

This project has no third-party dependencies and renders on the terminal.  
To compile:
```
make
```
To run:
```
./play path/to/rom.ch8
```
Compile and run the sanity tests (mostly for CI):
```
make test
```
To clean all object and executable files:
```
make clean
```

### 2.2 Usage

#### 2.2.1 Controls and UI

The keys along with their descriptions are listed on the right panel of the UI.
You can press each \[B\]racketed key to send an input.  

#### 2.2.2 Changing the controls

Chip8's hex keypad has been mapped to the keyboard in the
following way by default:
```
+---+---+---+---+       +---+---+---+---+
| 1 | 2 | 3 | C |       | 1 | 2 | 3 | 4 |
+---+---+---+---+       +---+---+---+---+
| 4 | 5 | 6 | D |       | q | w | e | r |
+---+---+---+---+  -->  +---+---+---+---+
| 7 | 8 | 9 | E |       | a | s | d | f |
+---+---+---+---+       +---+---+---+---+
| A | 0 | B | F |       | z | x | c | v |
+---+---+---+---+       +---+---+---+---+
```
You can edit the keys in
[keypad.hpp](https://github.com/leonmavr/chip-8/blob/master/include/keypad.hpp).

#### 2.2.2 .cfg files

Each rom (`.ch8` file in `roms` directory) is accompanied by a `.cfg` file. 
This describes each control key and sets the frequency each rom will run at. It
was necessary to set the frequency because Chip8 games used to run on different
machines over the decades. More detailed description of `.cfg` files can be
found at `roms/README.md`.

# 3. Features

- [x] CPU and renderer.
- [x] UI including controls and debugger view including pause, exit, and stepping key.
- [x] .cfg file for each ROM with editable presets (such as emulation frequency) and key descriptions for each rom.  Found at `roms/*.cfg`.
- [x] Togglable quirks \[1\] (XO-CHIP and SCHIP1.1 quirks are offered together).
- [x] Configurable keys.
- [x] CI.
- [ ] Sound; probably never going to implement this.

# 4. Architecture and implementation

### 4.1 System architecture

```
  <--- Main memory --->
                 Read                                   Registers
              <- only ->            Stack        0    1          15 
  +-----------+-------+        +-----------+     +--+ +--+       +--+
  |           |       |        |  |  |  |  |     |  | |  | . . . |  |
  |  Instr/s  | Fonts |        +-----------+     +--+ +--+       +--+
  |           |       |       0x18       ^            ^ 
  +-----------+-------+           ^      |            |
0xFFF     ^ 0x200   0x0           |     2 bytes     1 byte
          |   ^                   |
          |   |                   |
Program ------+                   |                      Delay    Sound
start     |                       |                      timer    time
          +-----------------------|------+               +---+    +---+
Program   |           Stack       |      |               |   |    |   |
counter   |           pointer     |    Index register    +---+    +---+
(PC)      |           (SP)        |    (I)               1 byte   1 byte
+----+    |           +----+      |    +----+
|    |----+           |    |------+    |    |
+----+                +----+           +----+
2 bytes               2 bytes          2 bytes

        Renderer
+----------------------+ ^
|                      | |
|                      | 32
|                      | |
+----------------------+ v
<-------- 64 ---------->
```


### 4.2 Implementation overview

Here is the idea of this implementation. Details are omitted. 
```
      +-------+
      | start |
      +-------+
          |
          +------>---------------------+
          |                            |
          +------>--------+            |
          |               |            |
          |          +~~~~~~~~~+  +~~~~~~~~~~+
      +---------+    | Delay & |  | Key      |
      | Set PC  |    | sound   |  | listener |
      +---------+    | thread  |  | thread   |
          |          +~~~~~~~~~+  +~~~~~~~~~~+
          v               |            |
          +-----<-------+ |            |
          |             | +-->--+--<---+
      +-------+         |       |
      | Fetch |         ^       x
      +-------+         |      / \ 
          |             |     /   \
     instruction        |    /     \
          v             +---x  End  x----------+
      +--------+          n  \ loop/   y       |
      | Decode |              \ ? /            |
      +--------+               \ /             |
          |                     x              v
        opcode                  |              |
          v                     |              |
      +---------+               |      +--------------+
      | Execute |               |      | Join threads |
      +---------+               |      +--------------+
          |                     |              |
          v                     ^              v
          |                     |              |
      +--------+                |              |
      | Render |                |              |
      +--------+                |              |
          |                     |              |
          +--------->-----------+              |
                                               |
          +------------------------------------+
          |
          v 
       +-----+
       | End |
       +-----+
```
The emulator uses 3 threads in total. More detailed comments on how the fetch-
decode-execute cycle works are found in the source file `chip8.cpp`.
Instructions are implemented in a neat but non-optimised instruction table.
This suffices since we typically run on a 300 to 2000 instruction per second
clock. More comprehensive techninal resources are found in the reference
section.

# 5. Improvements/fixed issues

- [x] Get rid of excessive screen flicker ([f27bd6d](https://github.com/leonmavr/chip-8/commit/f27bd6d0bb2c32fe9879f90c9354cb34d11e9438))

# References and credits
1. [Steffen Schumann's instruction table](https://chip8.gulrak.net/)
2. [Eric Bryntse's overview](http://devernay.free.fr/hacks/chip8/schip.txt)
3. [Matthew Mikolay's instruction set writeup](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set#notes)
4. [Columbia uni's technical guide](https://www.cs.columbia.edu/~sedwards/classes/2016/4840-spring/reports/Chip8.pdf)
5. [emudev subreddit](https://reddit.com/r/emudev) for the inspiration
6. [dmatlack](github.com/dmatlack) for the [roms](https://github.com/dmatlack/chip8/tree/master/roms/games)
