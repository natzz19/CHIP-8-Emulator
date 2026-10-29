CHIP-8 Emulator
===============

A CHIP-8 interpreter written in C, using SDL2 for video and input.
I wrote it to learn how basic emulators work. It implements the standard
CHIP-8 instruction set and loads plain .ch8 ROM files.

Not done:
  - No sound. The sound timer counts down but nothing beeps.
  - Fixed 600x200 window, no scaling options or colors.
  - Original (COSMAC VIP) quirks are not selectable. Some ROMs written for
    other interpreters may misbehave.


Building
--------

Needs a C11 compiler, CMake (3.15+) and the SDL2 development files.

  Debian/Ubuntu:   sudo apt install build-essential cmake libsdl2-dev

  mkdir build && cd build
  cmake ..
  make

This produces ./CHIP_8


Usage
-----

  ./CHIP_8 <rom file>

ROMs are loaded at 0x200 and can be up to 3584 bytes. ROMs are not included.


Keys
----

The original 4x4 hex keypad is mapped onto the left side of a QWERTY
keyboard:

  Keypad            Keyboard

  1 2 3 C           1 2 3 4
  4 5 6 D           Q W E R
  7 8 9 E           A S D F
  A 0 B F           Z X C V

  Esc     quit
  +       speed up (100 instructions/sec)
  0       slow down (100 instructions/sec, minimum 100)

Default speed is 600 instructions per second. The display and timers
run at 60 Hz.


Files
-----

  main.c      SDL setup, main loop, input, drawing
  chip8.c     instruction decoding and execution, timers
  chip8.h     machine state
  fonts.h     built-in hex digit sprites
