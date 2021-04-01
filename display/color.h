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

#define color_white   ((color_t){.r = 255, .g = 255, .b = 255})
#define color_black   ((color_t){.r = 0, .g = 0, .b = 0})
#define color_red     ((color_t){.r = 255, .g = 0, .b = 0})
#define color_green   ((color_t){.r = 0, .g = 255, .b = 0})
#define color_blue    ((color_t){.r = 0, .g = 0, .b = 255})
#define color_purple  ((color_t){.r = 255, .g = 0, .b = 255})
#define color_orange  ((color_t){.r = 255, .g = 255, .b = 0})
#define color_yellow  ((color_t){.r = 255, .g = 128, .b = 0})
#define color_cyan    ((color_t){.r = 0, .g = 128, .b = 255})
#define color_magenta ((color_t){.r = 255, .g = 0, .b = 128})
#define color_gray    ((color_t){.r = 64, .g = 64, .b = 64})

rgb888_t RGB888(rgb565_t rgb);

rgb565_t RGB565(rgb888_t rgb);

#endif
