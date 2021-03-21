#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include <stdint.h>

#include "gui/gui_base.h"
#include "utils/geometry.h"

#define GUI_BUTTON_WIDTH_MIN 1
#define GUI_BUTTON_WIDTH_MAX 1024
#define GUI_BUTTON_HEIGHT_MIN 1
#define GUI_BUTTON_HEIGHT_MAX 1024

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    rect_t _dim;
    char* _text;
} gui_button_t;

#endif