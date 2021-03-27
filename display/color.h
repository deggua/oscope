#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

#include "utils/geometry.h"

typedef union {
    struct {
        uint16_t b : 5;
        uint16_t g : 6;
        uint16_t r : 5;
    };
    uint16_t rgb;
} rgb565_t;

typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    uint32_t rgb;
} rgb888_t;

typedef rgb888_t color_t;

extern const color_t color_white, color_black, color_red, color_green, color_blue, color_purple, color_orange, color_yellow,
    color_cyan, color_magenta;

rgb888_t RGB888(rgb565_t rgb);

rgb565_t RGB565(rgb888_t rgb);

#endif
