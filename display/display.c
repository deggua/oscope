#include "display/display.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "utils/geometry.h"

void SCR_DrawPixel(int32_t posx, int32_t posy, rgb888_t rgb) {
    point_t  coordReal = ConvertCoord_VirtualToReal(posx, posy);
    rgb565_t colorReal = RGB565(rgb);

    screen.pix[coordReal.x][coordReal.y] = colorReal;
    return;
}

static point_t ConvertCoord_VirtualToReal(int32_t posx, int32_t posy) {
    point_t ret;
    ret.x = screen.res.w - posx;
    ret.y = screen.res.h - posy;

    return ret;
}

static rgb888_t RGB888(rgb565_t rgb) {
    rgb888_t ret;
    ret.r = rgb.r << 3;
    ret.g = rgb.g << 2;
    ret.b = rgb.b << 3;

    return ret;
}

static rgb565_t RGB565(rgb888_t rgb) {
    rgb565_t ret;
    ret.r = rgb.r >> 3;
    ret.g = rgb.g >> 2;
    ret.b = rgb.b >> 3;

    return ret;
}