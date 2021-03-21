#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <stdint.h>

#include "utils/geometry.h"
#include "gui/gui_base.h"

#define GUI_WINDOW_WIDTH_MIN 1
#define GUI_WINDOW_WIDTH_MAX 1024
#define GUI_WINDOW_HEIGHT_MIN 1
#define GUI_WINDOW_HEIGHT_MAX 1024

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    rect_t _dim;
} gui_window_t;

#endif