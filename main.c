#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "fonts.h"

void handle_input(Chip8 *data, SDL_Event *event);
void render_screen(SDL_Renderer *renderer, SDL_Texture *texture, Chip8 *data);
void delay_freq(u32 start_tick);


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "usage: %s [ROM file]\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    Chip8 data;
    memset(&data, 0, sizeof(Chip8));
    memcpy(&data.memory[0], fonts, sizeof(fonts)); // load all 16 fonts

    FILE *rom = fopen(argv[1], "rb");
    if (rom == NULL) {
        perror("fopen");
        return 1;
    }


    fseek(rom, 0, SEEK_END);
    const u32 rom_size = ftell(rom);
    rewind(rom);
    if (rom_size > MAX_ROM) {
        fprintf(stderr, "ROM is too big! (%u bytes)\n", rom_size);
        return 1;
    }

    fread(&data.memory[512], 1, rom_size, rom);
    fclose(rom);

    data.pc = 512; // set program counter
    data.instFreq = 600;


/* Initialize screen */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_ERROR: %s\n", SDL_GetError());
        return 1;
    }

    int w = 600;
    int h = 200;
    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w, h,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) goto CLEANUP_EXIT;

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (renderer == NULL) goto CLEANUP_EXIT;

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, /* Tells GPU we will change the pixels continuously */
        64, 32
    );
    if (texture == NULL) goto CLEANUP_EXIT;


    SDL_Event event;
    bool quit = false;

/* Main Game Loop */
    while (!quit) {
        u32 start_tick = SDL_GetTicks();

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) quit = true;
            handle_input(&data, &event);
        }

        for (int i = 0; i < data.instFreq / 60; i++)
            execute_instructions(&data);

        update_timers(&data);

        if (data.draw_flag) {
            render_screen(renderer, texture, &data);
            data.draw_flag = false;
        }

        delay_freq(start_tick);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    return 0;


    CLEANUP_EXIT:
        fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
        if (window)     SDL_DestroyWindow(window);
        if (renderer)   SDL_DestroyRenderer(renderer);
        if (texture)    SDL_DestroyTexture(texture);
        SDL_Quit();
        return 1;


}


void render_screen(SDL_Renderer *renderer, SDL_Texture *texture, Chip8 *data) {

    for (int i = 0; i < 64 * 32; i++) {
        if (data->display[i] == 1) {
            data->pixel_buff[i] = WHITE;
        } else {
            data->pixel_buff[i] = BLACK;
        }
    }
    SDL_UpdateTexture(
        texture,
        NULL,
        data->pixel_buff,
        64 * sizeof(u32)
    );
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

}

void delay_freq(u32 start_tick) {
    u32 frame_time = SDL_GetTicks() - start_tick;
    if (frame_time < 16)
        SDL_Delay(16 - frame_time);
}

void handle_input(Chip8 *data, SDL_Event *event) {

    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        u8 state = (event->type == SDL_KEYDOWN) ? 1: 0;

        switch (event->key.keysym.sym) {
            case SDLK_1: data->keypad[0x1] = state; break;
            case SDLK_2: data->keypad[0x2] = state; break;
            case SDLK_3: data->keypad[0x3] = state; break;
            case SDLK_4: data->keypad[0xC] = state; break;

            case SDLK_q: data->keypad[0x4] = state; break;
            case SDLK_w: data->keypad[0x5] = state; break;
            case SDLK_e: data->keypad[0x6] = state; break;
            case SDLK_r: data->keypad[0xD] = state; break;

            case SDLK_a: data->keypad[0x7] = state; break;
            case SDLK_s: data->keypad[0x8] = state; break;
            case SDLK_d: data->keypad[0x9] = state; break;
            case SDLK_f: data->keypad[0xE] = state; break;

            case SDLK_z: data->keypad[0xA] = state; break;
            case SDLK_x: data->keypad[0x0] = state; break;
            case SDLK_c: data->keypad[0xB] = state; break;
            case SDLK_v: data->keypad[0xF] = state; break;

            case SDLK_ESCAPE: exit(0);   break;

            case SDLK_PLUS:
                if (state) data->instFreq += 100;
                break;
            case SDLK_0:
                if (state) data->instFreq -= 100;
                if (data->instFreq < 100) data->instFreq = 100;
                break;
        }
    }

}