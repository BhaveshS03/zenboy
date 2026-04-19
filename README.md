# zgboy

A Game Boy/Game Boy Color emulator written from scratch in C++20 and SDL2. It is built to simulate accurate hardware mechanics, timing synchronizations, and Game Boy memory mappings.

## Features

- **Cycle and Dot Accurate Timing:** Coordinates components leveraging simulated T-Cycles logic.
- **Graphic Modes:** Handles Backgrounds, Windows, and Objects across DMG mapping configurations.
- **OAM Space DMA:** Mimics Direct Memory Access correctly rendering and suspending hardware bus accesses.
- **Mappers (MBC):** Support for `MBC1`, `MBC2`, `MBC3` (RTC stubbed), and `MBC5`, alongside `ROM ONLY` configurations.

- **SRAM Persistence:** In-game saves are supported. `.sav` files are dumped exactly next to your `.gb` rom upon exiting. 

## Build Requirements

- C++20 compatible compiler (MSVC, GCC, Clang)
- CMake 3.20+
- (SDL2 is automatically fetched by CMake)

### Compiling on Windows/Linux/macOS

Generate and build the project through standard CMake execution.

```bash
# Generate build configuration
cmake -B build -S .

# Build the emulator binary
cmake --build build --config Release
```

The resulting executable will be placed in `build/bin/` as `zgboy` (or `zgboy.exe` on Windows).

## Usage

Simply run the compiled executable from your terminal passing the absolute or relative path to the `.gb` or `.gbc` ROM.

```bash
./build/bin/zgboy path/to/rom.gb
```

## Controls

The emulator binds PC keyboard inputs accurately mimicking a standard Game Boy interface.

| Game Boy | PC Keyboard Mapping |
| :--- | :--- |
| **D-Pad Right** | Right Arrow |
| **D-Pad Left** | Left Arrow |
| **D-Pad Up** | Up Arrow |
| **D-Pad Down** | Down Arrow |
| **A Button** | `X` Key |
| **B Button** | `Z` Key |
| **Select** | `Backspace` |
| **Start** | `Enter` / `Return`|

To quit the emulator, simply close the SDL rendering window. If your game modifies SRAM, the save data will automatically be flushed to disk on completion.
