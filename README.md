# ZenBoy – Game Boy Emulator

ZenBoy is a work-in-progress **Game Boy emulator** written in **C++17**.  
It currently supports CPU, memory, timers, and instruction set emulation, with plans to add GPU and audio support in the future.  

---

## Features
- CPU instruction set emulation (8-bit Game Boy Z80-like CPU).
- Memory management with cartridge loading.
- Timer and interrupt handling.
- Basic SDL2 integration for display & input (GPU rendering in progress).
- Verified using test ROMs and simple homebrew games.

---

## Build Instructions

### Prerequisites
- **CMake ≥ 3.10**
- **C++17 compatible compiler** (GCC, Clang, or MSVC)
- **SDL2** (for display and input handling)

### Clone the Repository
```bash
git clone https://github.com/BhaveshS03/zenboy.git
cd zenboy
mkdir build && cd build
cmake ..
make

./emulator path/to/game.gb
```
