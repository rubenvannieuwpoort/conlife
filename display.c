#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int scale = 1;

bool dopoll() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
    }
    return true;
}

SDL_Window *window;
SDL_Renderer *renderer;
int init(const char *title, int window_width, int window_height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window and renderer
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              window_width, window_height, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // // Allocate and initialize a grayscale pixel array
    // uint8_t *pixels = (uint8_t *)malloc(GRID_WIDTH * GRID_HEIGHT * sizeof(uint8_t));
    // if (!pixels) {
    //     printf("Memory allocation failed.\n");
    //     SDL_DestroyRenderer(renderer);
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return 1;
    // }

    return 0;
}

void display(int width, int height, uint8_t *grid1, uint8_t *grid2, uint8_t *grid3) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear the screen
    SDL_RenderClear(renderer);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t grayscale_value = 255 * grid1[y * width + x];
            SDL_SetRenderDrawColor(renderer, grayscale_value, grayscale_value, grayscale_value, 255);
            SDL_Rect rect = { x * scale, y * scale, scale, scale };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t grayscale_value = 255 * grid2[y * width + x];
            SDL_SetRenderDrawColor(renderer, grayscale_value, grayscale_value, grayscale_value, 255);
            SDL_Rect rect = { (x + width) * scale, y * scale, scale, scale };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t grayscale_value = grid3[y * width + x];
            SDL_SetRenderDrawColor(renderer, grayscale_value, grayscale_value, grayscale_value, 255);
            SDL_Rect rect = { (x + 2 * width) * scale, y * scale, scale, scale };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}


void deinit() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}