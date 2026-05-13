//
// Created by nati on 5/12/26.
//

#ifndef CHIP_8_CHIP8_H
#define CHIP_8_CHIP8_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_MEM 4096
#define MAX_ROM (MAX_MEM - 512)
#define MAX_STACK 256

#define INSTRUCTIONS_FREQ 10

#define BLACK 0x00000000
#define WHITE 0xFFFFFFFF

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct {
    u8  memory[MAX_MEM];
    u8  v[16];
    u16 pc; // Program Counter
    u16 i;  // Index

    u16 stack[MAX_STACK];
    u8  sp;  // Stack Pointer

    u8  dt;  // Delay Timer
    u8  st;  // Sound Timer

    u8  keypad         [16];
    u8  display   [64 * 32];
    u32 pixel_buff[64 * 32]; // for SDL
    bool draw_flag;

    u32 instFreq;
} Chip8;


void update_timers(Chip8 *data);
void execute_instructions(Chip8 *data);


#endif //CHIP_8_CHIP8_H
