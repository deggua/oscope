#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "utils/geometry.h"

typedef struct {
    uint8_t r,g,b;
} color_t;

typedef struct {
    rect_t res;
    color_t* pix;
} screen_t;

#endif