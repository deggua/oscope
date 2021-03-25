#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

#include "utils/geometry.h"

#define resWidth (480)
#define resHeight (272)


/* 
typedef uint16_t rgb565_t
void RGB565_SetRGB(rgb565_t* pix, uint8_t r, uint8_t g, uint8_t b) {
	pix->color = ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}
*/

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
    rect_t res;
    volatile rgb565_t pix[resWidth][resHeight];
} screen_t;

typedef rgb888_t color_t;

volatile screen_t screen = {
	.pix = {0},
	.res.w = resWidth,
	.res.h = resHeight
};

void SCR_DrawPixel(int32_t posx, int32_t posy, rgb888_t rgb);

static point_t ConvertCoord_VirtualToReal(int32_t posx, int32_t posy);
static rgb888_t RGB888(rgb565_t rgb);
static rgb565_t RGB565(rgb888_t rgb);

#endif
