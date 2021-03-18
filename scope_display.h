#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t r,g,b;
} color_t;

typedef struct {
    int32_t x,y;
} point_t;

typedef struct {
    int32_t width, height;
} rect_t;
