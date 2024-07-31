### Roms directory

Each ROM (.ch8 file) is optionally accompanied by a config file (.cfg).  
The config file describes the controls and the CPU speed so they can be
displayed in the UI. It can also include comments, whose lines are preceeded by `#`.  

Here's an example config:
```
# Game objective
0x4: Move left
0x6: Move right
0x5: Shoot
```
