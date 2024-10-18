#ifndef IMAGE_H
#define IMAGE_H


#include <stdint.h>


typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t *data;
} Image;

Image *load(const char *);

void unload(Image *);


#endif
