#include "color.h"

const color_t color_white   = {.r = 255, .g = 255, .b = 255};
const color_t color_black   = {.r = 0, .g = 0, .b = 0};
const color_t color_red     = {.r = 255, .g = 0, .b = 0};
const color_t color_green   = {.r = 0, .g = 255, .b = 0};
const color_t color_blue    = {.r = 0, .g = 0, .b = 255};
const color_t color_purple  = {.r = 255, .g = 0, .b = 255};
const color_t color_orange  = {.r = 255, .g = 255, .b = 0};
const color_t color_yellow  = {.r = 255, .g = 128, .b = 0};
const color_t color_cyan    = {.r = 0, .g = 128, .b = 255};
const color_t color_magenta = {.r = 255, .g = 0, .b = 128};
const color_t color_gray = {.r = 64, .g = 64, .b = 64};

rgb888_t RGB888(rgb565_t rgb) {
    rgb888_t ret;
    ret.r = rgb.r << 3;
    ret.g = rgb.g << 2;
    ret.b = rgb.b << 3;

    return ret;
}

rgb565_t RGB565(rgb888_t rgb) {
    rgb565_t ret;
    ret.r = rgb.r >> 3;
    ret.g = rgb.g >> 2;
    ret.b = rgb.b >> 3;

    return ret;
}
