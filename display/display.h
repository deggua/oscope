#ifndef DISPLAY_H
#define DISPLAY_H

/* --- Includes --- */
#include <stdbool.h>
#include <stdint.h>

#include "utils/geometry.h"

/* --- Defines --- */
#define resWidth  (480)
#define resHeight (272)

/*
typedef uint16_t rgb565_t
void RGB565_SetRGB(rgb565_t* pix, uint8_t r, uint8_t g, uint8_t b) {
    pix->color = ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}
*/

/* --- Types, Structs, Unions --- */
typedef union {
    struct {
        uint16_t r : 5;
        uint16_t g : 6;
        uint16_t b : 5;
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

typedef struct {
    rect_t            res;
    volatile rgb565_t pix[resWidth][resHeight];
} screen_t;

typedef rgb888_t color_t;

/* --- Global Variables --- */
volatile screen_t screen = {.pix = {0}, .res.w = resWidth, .res.h = resHeight};

/* --- Public Functions --- */
void SCR_DrawPixel(int32_t x0, int32_t y0, color_t rgb);

void SCR_DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, color_t rgb);

void SCR_DrawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool fill, color_t rgb);

void SCR_DrawCircle(int32_t x0, int32_t y0, int32_t radius, bool fill, color_t rgb);

void SCR_DrawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool fill, color_t rgb);

void SCR_DrawString(int32_t x0, int32_t y0, const char* text, int32_t scale, color_t rgb);

void SCR_DrawChar(int32_t x0, int32_t y0, char cc, int32_t scale, color_t rgb);
#endif
