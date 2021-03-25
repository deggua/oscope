#include "display/display.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display/fonts/renew_font.h"
#include "utils/geometry.h"

#define FONT_CHARS  96UL
#define FONT_HEIGHT 8UL
#define FONT_WIDTH  (sizeof(font) / FONT_CHARS)

/* --- Private Functions --- */
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

static void DrawLineLow(int32_t x0, int32_t y0, int32_t x1, int32_t y1, color_t rgb) {
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;

    int32_t yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }

    int32_t diff = 2 * dy - dx;
    int32_t yy   = y0;

    for (int32_t xx = x0; xx <= x1; xx++) {
        SCR_DrawPixel(xx, yy, rgb);

        if (diff > 0) {
            yy = yy + yi;
            diff += 2 * (dy - dx);
        } else {
            diff += 2 * dy;
        }
    }

    return;
}

static void DrawLineHigh(int32_t x0, int32_t y0, int32_t x1, int32_t y1, color_t rgb) {
    int32_t dx = x1 - x0;
    int32_t dy = y1 - y0;

    int32_t xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }

    int32_t diff = 2 * dx - dy;
    int32_t xx   = x0;

    for (int32_t yy = y0; yy <= y1; yy++) {
        SCR_DrawPixel(xx, yy, rgb);

        if (diff > 0) {
            xx = xx + xi;
            diff += 2 * (dx - dy);
        } else {
            diff += 2 * dx;
        }
    }

    return;
}

static void DrawLineHorizontal(int32_t x0, int32_t x1, int32_t ypos, color_t rgb) {
    int32_t xstart, xend;
    if (x0 < x1) {
        xstart = x0;
        xend   = x1;
    } else {
        xstart = x1;
        xend   = x0;
    }

    for (int32_t xx = xstart; xx <= xend; xx++) {
        SCR_DrawPixel(xx, ypos, rgb);
    }
}

static void DrawLineVertical(int32_t y0, int32_t y1, int32_t xpos, color_t rgb) {
    int32_t ystart, yend;
    if (y0 < y1) {
        ystart = y0;
        yend   = y1;
    } else {
        ystart = y1;
        yend   = y0;
    }

    for (int32_t yy = ystart; yy <= yend; yy++) {
        SCR_DrawPixel(xpos, yy, rgb);
    }
}

static void Draw8CirclePoints(int32_t xc, int32_t yc, int32_t x0, int32_t y0, bool fill, color_t rgb) {
    if (fill) {
        // draw vertical lines connecting pixels
        // (???) maybe change these to directly call draw vertical lines
        SCR_DrawLine(xc + x0, yc + y0, xc + x0, yc - y0, rgb);
        SCR_DrawLine(xc - x0, yc + y0, xc - x0, yc - y0, rgb);
        SCR_DrawLine(xc + y0, yc + x0, xc + y0, yc - x0, rgb);
        SCR_DrawLine(xc - y0, yc + x0, xc - y0, yc - x0, rgb);
    } else {
        SCR_DrawPixel(xc + x0, yc + y0, rgb);
        SCR_DrawPixel(xc - x0, yc + y0, rgb);
        SCR_DrawPixel(xc - x0, yc - y0, rgb);
        SCR_DrawPixel(xc + x0, yc - y0, rgb);

        SCR_DrawPixel(xc + y0, yc + x0, rgb);
        SCR_DrawPixel(xc - y0, yc + x0, rgb);
        SCR_DrawPixel(xc - y0, yc - x0, rgb);
        SCR_DrawPixel(xc + y0, yc - x0, rgb);
    }

    return;
}

static void FillBottomFlatTriangle(point_t v0, point_t v1, point_t v2, color_t rgb) {
    float invslope1 = (v1.x - v2.x) / (v1.y - v0.y);
    float invslope2 = (v2.x - v0.x) / (v2.y - v0.y);

    float curx1 = v1.x;
    float curx2 = v1.x;

    for (int32_t yy = v0.y; yy <= v1.y; yy++) {
        SCR_DrawLine((int32_t)curx1, yy, (int32_t)curx2, yy, rgb);
        curx1 += invslope1;
        curx2 += invslope2;
    }
    return;
}

static void FillTopFlatTriangle(point_t v0, point_t v1, point_t v2, color_t rgb) {
    float invslope1 = (v2.x - v0.x) / (v2.y - v0.y);
    float invslope2 = (v2.x - v1.x) / (v2.y - v1.y);

    float curx1 = v2.x;
    float curx2 = v2.x;

    for (int32_t yy = v2.y; yy <= v0.y; yy++) {
        SCR_DrawLine((int32_t)curx1, yy, (int32_t)curx2, yy, rgb);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
    return;
}

/* --- Public Functions --- */
void SCR_DrawPixel(int32_t x0, int32_t y0, color_t rgb) {
    // check for invalid coord
    if (x0 < 0 || x0 > resWidth || y0 < 0 || y0 > resHeight) {
        return;
    }

    point_t  coordReal = ConvertCoord_VirtualToReal(x0, y0);
    rgb565_t colorReal = RGB565(rgb);

    screen.pix[coordReal.x][coordReal.y] = colorReal;
    return;
}

void SCR_DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, color_t rgb) {
    if (x0 == x1 && y0 == y1) {
        // single pixel lines
        SCR_DrawPixel(x0, y0, rgb);
    } else if (x0 == x1) {
        // vertical lines
        DrawLineVertical(y0, y1, x0, rgb);
    } else if (y0 == y1) {
        // horizontal lines
        DrawLineHorizontal(x0, x1, y0, rgb);
    } else {
        // general lines
        if (abs(y1 - y0) < abs(x1 - x0)) {
            if (x0 > x1) {
                DrawLineLow(x1, y1, x0, y0, rgb);
            } else {
                DrawLineLow(x0, y0, x1, y1, rgb);
            }
        } else {
            if (y0 > y1) {
                DrawLineHigh(x1, y1, x0, y0, rgb);
            } else {
                DrawLineHigh(x0, y0, x1, y1, rgb);
            }
        }
    }

    return;
}

void SCR_DrawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool fill, color_t rgb) {
    // check for single pixel rectangles
    if (x0 == x1 && y0 == y1) {
        SCR_DrawPixel(x0, y0, rgb);
        return;
    }

    if (fill) {
        point_t topleft, botright;
        if (x0 < x1) {
            topleft.x  = x0;
            botright.x = x1;
        } else {
            topleft.x  = x1;
            botright.x = x0;
        }

        if (y0 < y1) {
            topleft.y  = y0;
            botright.y = y1;
        } else {
            topleft.y  = y1;
            botright.y = y0;
        }

        for (int32_t xx = topleft.x; xx <= botright.x; xx++) {
            for (int32_t yy = topleft.y; yy <= botright.y; yy++) {
                SCR_DrawPixel(xx, yy, rgb);
            }
        }

    } else {
        // (???) maybe change these to use horizontal and vertical line draws
        SCR_DrawLine(x0, y0, x0, y1, rgb);
        SCR_DrawLine(x0, y0, x1, y0, rgb);
        SCR_DrawLine(x1, y1, x0, y1, rgb);
        SCR_DrawLIne(x1, y1, x1, y0, rgb);
    }

    return;
}

void SCR_DrawCircle(int32_t xc, int32_t yc, int32_t radius, bool fill, color_t rgb) {
    int32_t xx, yy;
    int32_t xchange, ychange;
    int32_t errRadius;

    xx = radius;
    yy = 0;

    xchange   = 1 - 2 * radius;
    ychange   = 1;
    errRadius = 0;

    while (xx >= yy) {
        Draw8CirclePoints(xc, yc, xx, yy, fill, rgb);

        yy++;
        errRadius += ychange;
        ychange += 2;

        if (2 * errRadius + xchange > 0) {
            xx--;
            errRadius += xchange;
            xchange += 2;
        }
    }

    return;
}

void SCR_DrawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool fill, color_t rgb) {
    if (fill) {
        // sort vertices
        point_t v0, v1, v2, vtemp;
        v0 = (point_t){.x = x0, .y = y0};
        v1 = (point_t){.x = x1, .y = y1};
        v2 = (point_t){.x = x2, .y = y2};

        if (v1.y <= v0.y) {
            vtemp = v0;
            v0    = v1;
            v1    = vtemp;
        }

        if (v2.y <= v1.y) {
            vtemp = v1;
            v1    = v2;
            v2    = vtemp;
        }

        if (v1.y <= v0.y) {
            vtemp = v0;
            v0    = v1;
            v1    = vtemp;
        }

        // decompose and draw
        if (v1.y == v2.y) {
            FillBottomFlatTriangle(v0, v1, v2, rgb);
        } else if (v0.y == v2.y) {
            FillTopFlatTriangle(v0, v1, v2, rgb);
        } else {
            point_t v3;
            v3.x = (int32_t)(v0.x + ((float)(v1.y - v0.y) / (float)(v2.y - v0.y)) * (v2.x - v0.x));
            v3.y = v1.y;
            FillBottomFlatTriangle(v0, v1, v3, rgb);
            FillTopFlatTriangle(v1, v3, v2, rgb);
        }

    } else {
        SCR_DrawLine(x0, y0, x1, y1, rgb);
        SCR_DrawLine(x0, y0, x2, y2, rgb);
        SCR_DrawLine(x1, y1, x2, y2, rgb);
    }

    return;
}

void SCR_DrawString(int32_t x0, int32_t y0, const char* str, int32_t scale, color_t rgb) {
    int32_t xx = x0;
    while (*str) {
        SCR_DrawChar(xx, y0, *str++, scale, rgb);
        xx += FONT_WIDTH * scale;
    }

    return;
}

void SCR_DrawChar(int32_t x0, int32_t y0, char cc, int32_t scale, color_t rgb) {
    cc &= 0x7F;
    if (cc < ' ') {
        cc = 0;
    } else {
        cc -= ' ';
    }

    const uint8_t* bmp = font[cc];

    for (int32_t ii = 0; ii < scale * FONT_WIDTH; ii++) {
        for (int32_t jj = 0; jj < scale * FONT_HEIGHT; jj++) {
            if (bmp[ii / scale] & (1 << (jj / scale))) {
                SCR_DrawPixel(x0 + ii, y0 + jj, rgb);
            }
        }
    }

    return;
}