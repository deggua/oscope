#include "color.h"

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
