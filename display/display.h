#ifndef DISPLAY_H
#define DISPLAY_H

/* --- Includes --- */
#include <stdbool.h>
#include <stdint.h>

#include "display/color.h"
#include "utils/geometry.h"

/* --- Defines --- */
#define SCR_RES_WIDTH  (480)
#define SCR_RES_HEIGHT (272)

#define FONT_CHARS  96UL
#define FONT_HEIGHT 8UL
#define FONT_WIDTH  7UL

/*
typedef uint16_t rgb565_t
void RGB565_SetRGB(rgb565_t* pix, uint8_t r, uint8_t g, uint8_t b) {
    pix->color = ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}
*/

/* --- Types, Structs, Unions --- */
typedef struct {
    rect_t            res;
    volatile rgb565_t pix[SCR_RES_WIDTH * SCR_RES_HEIGHT];
} screen_t;

/* --- Global Variables --- */
extern screen_t screen;

/* --- Public Functions --- */
void SCR_DrawPixel(int32_t x0, int32_t y0, color_t rgb);

void SCR_DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, color_t rgb);

void SCR_DrawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool fill, color_t rgb);

void SCR_DrawCircle(int32_t x0, int32_t y0, int32_t radius, bool fill, color_t rgb);

void SCR_DrawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, bool fill, color_t rgb);

void SCR_DrawString(int32_t x0, int32_t y0, const char* text, int32_t scale, color_t rgb);

void SCR_DrawChar(int32_t x0, int32_t y0, char cc, int32_t scale, color_t rgb);

void SCR_DrawHorizontalDashedLine(
    int32_t x0,
    int32_t x1,
    int32_t y0,
    int32_t dashed_len,
    int32_t blank_len,
    color_t rgb);

void SCR_DrawVerticalDashedLine(int32_t x0, int32_t y0, int32_t y1, int32_t dashed_len, int32_t blank_len, color_t rgb);

#endif
