#include "image.h"
#include "display.h"

#include <SDL2/SDL.h>
#include <stdio.h>

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>


#define TARGET_PATH "conway.raw"
#define WIDTH 707
#define HEIGHT 900

#define N 1

#define GAMMA 1.5

#define GLOBALGRIDSIZE (WIDTH * HEIGHT)
#define LOCALGRIDDEPTH (2 * N + 2)
#define LOCALGRIDLENGTH (2 * LOCALGRIDDEPTH + 1)
#define LOCALGRIDSIZE (LOCALGRIDLENGTH * LOCALGRIDLENGTH)
#define LOCALGRIDCENTER (LOCALGRIDSIZE / 2)

#define SUM_COEFFS 10
const uint8_t coeffs[9] = { 1, 1, 1, 1, 2, 1, 1, 1, 1 };

// #define SUM_COEFFS 16
// const uint8_t coeffs[9] = { 1, 2, 1, 2, 4, 2, 1, 2, 1 };

uint8_t gamma_encoded[SUM_COEFFS] = { 0 };  // will be filled

uint64_t local_loss[GLOBALGRIDSIZE];
uint8_t global_grid[GLOBALGRIDSIZE];
uint8_t global_final[GLOBALGRIDSIZE];
uint8_t *target;

uint64_t compute_local_loss(int x, int y, bool flip, uint8_t **result_grid);
void update(void);


int main(int argc, char *argv[]) {
    // Initialize gamma correction lookup table
    for (int i = 0; i < SUM_COEFFS; i++) {
        gamma_encoded[i] = (uint8_t)round(pow(i / (1.0 * SUM_COEFFS), 1.0 / GAMMA) * 255);
    }

    Image *image = load(TARGET_PATH);
    if (!image) {
        return 1;
    }

    assert(image->width == WIDTH);
    assert(image->height == HEIGHT);

    target = image->data;
    
    init("Approximate atavising", 3 * WIDTH, HEIGHT);

    // initialize grid
    srand(0);
    int i = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            global_grid[i++] = rand() & 1;
        }
    }

    // fill grid of local losses
    // this computation is very inefficient but we only do it once
    i = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            local_loss[i++] = compute_local_loss(x, y, false, NULL);
        }
    }

    while (dopoll()) {
        int pixel_batch_size = 10000;
        for (int i = 0; i < pixel_batch_size; i++) {
            int x = rand() % WIDTH, y = rand() % HEIGHT;
            uint8_t *result_grid;
            uint64_t candidate_local_loss = compute_local_loss(x, y, true, &result_grid);
            if (candidate_local_loss <= local_loss[WIDTH * y + x]) {
                global_grid[WIDTH * y + x] = 1 - global_grid[WIDTH * y + x];
                local_loss[WIDTH * y + x] = candidate_local_loss;

                for (int dx = -N; dx <= N; dx++) {
                    for (int dy = -N; dy <= N; dy++) {
                        int sx = (x + dx + WIDTH) % WIDTH, sy = (y + dy + HEIGHT) % HEIGHT;
                        global_final[WIDTH * sy + sx] = result_grid[LOCALGRIDCENTER + LOCALGRIDLENGTH * dy + dx];
                    }
                }
            }
        }

        update();
    }

    unload(image);
    deinit();

    return 0;
}

uint8_t filtered(uint8_t *local_grid, int i) {
    int sum = (coeffs[0] * local_grid[i - LOCALGRIDLENGTH - 1] + coeffs[1] * local_grid[i - LOCALGRIDLENGTH] + coeffs[2] * local_grid[i - LOCALGRIDLENGTH + 1]
             + coeffs[3] * local_grid[i - 1]                   + coeffs[4] * local_grid[i]                   + coeffs[5] * local_grid[i + 1]
             + coeffs[6] * local_grid[i + LOCALGRIDLENGTH - 1] + coeffs[7] * local_grid[i + LOCALGRIDLENGTH] + coeffs[8] * local_grid[i + LOCALGRIDLENGTH + 1]);
    return gamma_encoded[sum];
}

uint8_t local_grid1[LOCALGRIDSIZE], local_grid2[LOCALGRIDSIZE];
uint64_t compute_local_loss(int x, int y, bool flip, uint8_t **resultgrid) {
    uint8_t *local_grid = &local_grid1[0], *next_local_grid = &local_grid2[0];

    // fill grid
    for (int dx = -LOCALGRIDDEPTH; dx <= LOCALGRIDDEPTH; dx++) {
        for (int dy = -LOCALGRIDDEPTH; dy <= LOCALGRIDDEPTH; dy++) {
            int sx = (x + dx + WIDTH) % WIDTH, sy = (y + dy + HEIGHT) % HEIGHT;
            uint8_t pixel = global_grid[WIDTH * sy + sx];
            local_grid[LOCALGRIDCENTER + LOCALGRIDLENGTH * dy + dx] = pixel;
        }
    }

    if (flip) {
        local_grid[LOCALGRIDCENTER] = 1 - local_grid[LOCALGRIDCENTER];
    }

    // play N steps of game of life
    for (int k = 1; k <= N; k++) {
        int depth = LOCALGRIDDEPTH - k;
        for (int dy = -depth; dy <= depth; dy++) {
            for (int dx = -depth; dx <= depth; dx++) {
                int i = LOCALGRIDCENTER + LOCALGRIDLENGTH * dy + dx;
                int neighbors = local_grid[i - LOCALGRIDLENGTH - 1] + local_grid[i - LOCALGRIDLENGTH] + local_grid[i - LOCALGRIDLENGTH + 1]
                              + local_grid[i - 1]                                                     + local_grid[i + 1]
                              + local_grid[i + LOCALGRIDLENGTH - 1] + local_grid[i + LOCALGRIDLENGTH] + local_grid[i + LOCALGRIDLENGTH + 1]; 
                
                if (neighbors < 2 || neighbors > 3) {
                    next_local_grid[i] = 0;
                } else if (neighbors == 2) {
                    next_local_grid[i] = local_grid[i];
                } else if (neighbors == 3) {
                    next_local_grid[i] = 1;
                }
            }
        }

        uint8_t *temp = local_grid;
        local_grid = next_local_grid;
        next_local_grid = temp;
    }

    // recompute local loss
    uint64_t local_loss = 0;
    for (int dx = -(N + 1); dx <= N + 1; dx++) {
        for (int dy = -(N + 1); dy <= N + 1; dy++) {
            int i = LOCALGRIDCENTER + dy * LOCALGRIDLENGTH + dx;
            int sx = (x + dx + WIDTH) % WIDTH, sy = (y + dy + HEIGHT) % HEIGHT;
            int64_t diff = (int64_t)(target[WIDTH * sy + sx]) - filtered(local_grid, i);
            local_loss += (uint64_t)(diff * diff);
        }
    }

    if (resultgrid) {
        *resultgrid = local_grid;
    }

    return local_loss;    
}

void update(void) {
    display(WIDTH, HEIGHT, global_grid, global_final, target);
}