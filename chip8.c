#include "chip8.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void update_timers(Chip8 *data) {

    if (data->dt > 0) data->dt--;
    if (data->st > 0) data->st--;
}

void execute_instructions(Chip8 *data) {

    // CPU runs at 600Hz

    u16 inst = (data->memory[data->pc] << 8) | data->memory[data->pc + 1]; // implicit type-conversion
    data->pc += 2; // increment directly

    const u8 opcode = (inst & 0xF000) >> 12;
    const u8 x = (inst & 0x0F00) >> 8;
    const u8 y = (inst & 0x00F0) >> 4;
    const u8 n = (inst & 0x000F);
    const u8 nn = (inst & 0x00FF);
    const u16 nnn = (inst & 0x0FFF);

    switch (opcode) {

    /* general purpose instructions */
        case 0x1:   // jump (set pc) to nnn
            data->pc = nnn;
            break;

        case 0x6:   // set v[x] to nn
            data->v[x] = nn;
            break;

        case 0x7:   // adds nn to v[x]
            data->v[x] += nn;
            break;

        case 0xB:   // jump/set PC to nnn + V0
            data->pc = nnn + data->v[0];
            break;

    /* Operational instructions between Vx and Vy*/
        case 0x8: {
            u8 flag;
            u8 result;
            switch (n) { // last nibble

                case 0x0:   // sets v[x] to v[y]
                    data->v[x] = data->v[y];
                    break;

                case 0x1:   // bitwise OR
                    data->v[x] |= data->v[y];
                    break;

                case 0x2:   // bitwise AND
                    data->v[x] &= data->v[y];
                    break;

                case 0x3:   // bitwise XOR
                    data->v[x] ^= data->v[y];
                    break;

                case 0x4:   // Vx = Vx + Vy; VF = 1 if result > 255
                    u32 sum = data->v[x] + data->v[y];
                    data->v[0xF] = (sum > 255) ? 1 : 0;
                    data->v[x] = (u8)sum;
                    break;

                case 0x5:   // VF = 1 if Vx >= Vy; Vx = Vx - Vy
                    /* avoid edge-case where x or y is 0xF */
                    flag = (data->v[x] >= data->v[y]) ? 1 : 0;
                    result = data->v[x] - data->v[y];

                    data->v[x] = result;
                    data->v[0xF] = flag;
                    break;

                case 0x6: // VF = 1 if least-significant-bit of Vx is 1; Vx / 2
                    data->v[0xF] = data->v[x] & 0x1;
                    data->v[x] >>= 1;
                    break;

                case 0x7:
                    /* avoid edge-case where x or y is 0xF */
                    flag = (data->v[y] >= data->v[x]) ? 1 : 0;
                    result = data->v[y] - data->v[x];

                    data->v[x] = result;
                    data->v[0xF] = flag;
                    break;

                case 0xE: // VF = 1 if most-significant-bit of Vx is 1; Vx * 2
                    data->v[0xF] = (data->v[x] >> 7) & 0x1;
                    data->v[x] <<= 1;
                    break;

                default: // in case n is invalid
                    fprintf(stderr, "unknown operational code: 8xy'%d'", n);
                    exit(EXIT_FAILURE);
                    break;
            }
        }
        break; // break 0x8
    /* Conditional instructions */
        case 0x3:   // skip next instruction if Vx == nn
            if (data->v[x] == nn) data->pc += 2;
            break;

        case 0x4:   // skip next instruction if Vx != nn
            if (data->v[x] != nn) data->pc += 2;
            break;

        case 0x5: // skip next instruction if Vx == Vy
            if (data->v[x] == data->v[y]) data->pc += 2;
            break;

        case 0x9:   // skip next instruction if Vx != nn
            if (data->v[x] != data->v[y]) data->pc += 2;
            break;


    /* Subroutines & Clear Display */
        case 0x2:   // jump to subroutine
            data->stack[data->sp++] = data->pc; // store current PC on stack
            data->pc = nnn;
            break;

        case 0x0:
            if (nn == 0xE0) {
                // clear display
                memset(&data->display, 0, sizeof(data->display));
                memset(&data->pixel_buff, 0, sizeof(data->pixel_buff));
                data->draw_flag = true;
            }

            else if (nn == 0xEE) {
                // return from subroutine
                data->pc = data->stack[--data->sp];
            }
            break;

    /* Index register & Addressing */
        case 0xA:   // jump/set I to nnn
            data->i = nnn;
            break;

        case 0xF:
            switch (nn) { // last two nibbles

            case 0x1E:      // set I += Vx
                    data->i += data->v[x];
                    break;

            case 0x29:      // set I to font specified in Vx
                    data->i = data->v[x] * 5;
                    break;

            case 0x55:      // store V0-Vx in mem starting from I
                    for (int c = 0; c <= x; c++)
                        data->memory[data->i + c] = data->v[c];
                    break;

            case 0x65:      // store values in mem I-(I+x) into V0-Vx
                    for (int c = 0; c <= x; c++)
                        data->v[c] = data->memory[data->i + c];
                    break;

        /* Timers & sound */
            case 0x07:      // set Vx to delay timer
                    data->v[x] = data->dt;
                    break;

            case 0x15:      // set delay timer to Vx
                    data->dt = data->v[x];
                    break;

            case 0x18:      // set sound timer to Vx
                    data->st = data->v[x];
                    break;

        /* Binary-Coded Decimal */
            case 0x33:
                    data->memory[data->i] =      data->v[x] / 100;
                    data->memory[data->i + 1] = (data->v[x] / 10) % 10;
                    data->memory[data->i + 2] =  data->v[x] % 10;
                    break;

        /* Wait For Key Press*/
            case 0x0A:
                    bool found = false;
                    for (int c = 0; c < 16; c++) {
                        if (data->keypad[c] == 1) {
                            data->v[x] = c;
                            found = true;
                            break;
                        }
                    }
                    if (!found) data->pc -= 2; // loop current instruction until pressed-key found
                    break;

            default: // in case nn is invalid
                    fprintf(stderr, "Unknown Opcode: 0x%04X at Address: 0x%04X\n", inst, data->pc - 2);
                    exit(EXIT_FAILURE);
                    break;
            }
        break; // break 0xF
    /* Rand Generator */
        case 0xC:
            data->v[x] = (rand() % 256) & nn;
            break;

    /* Input Handling */
        case 0xE:
            if (nn == 0x9E) {
                if (data->keypad[data->v[x]] == 1) data->pc += 2;
            }
            if (nn == 0xA1) {
                if (data->keypad[data->v[x]] == 0) data->pc += 2;
            }
            break;

    /* Display & Rendering */
        case 0xD: {
            u8 axisX = data->v[x] % 64;
            u8 axisY = data->v[y] % 32;
            data->v[0xF] = 0; // assume no current collisions

            for (int row = 0; row < n; row++) {

                u8 pixel = data->memory[data->i + row];

                for (int bit = 0; bit < 8; bit++) {

                    u8 finalX = axisX + bit;
                    u8 finalY = axisY + row;
                    if (finalX >= 64 || finalY >= 32) continue; // dont force-wrap

                    u8 bit_val = (pixel >> (7 - bit)) & 0x01; // retrieve curr-bit
                    if (bit_val == 1) {

                        u16 index = (finalY * 64) + finalX;
                        if (data->display[index] == 1) { // check collision
                            data->v[0xF] = 1;
                        }

                        data->display[index] ^= 1;
                    }
                }
            }
            data->draw_flag = true;
            break;
        }

        default:
            fprintf(stderr, "unknown opcode '%d'", opcode);
            exit(EXIT_FAILURE);
            break;
    } // Instruction set ends

}
