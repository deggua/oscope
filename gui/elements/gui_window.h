#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <stdint.h>

#include "gui/gui_base.h"
#include "utils/geometry.h"

#define GUI_WINDOW_WIDTH_MIN  1
#define GUI_WINDOW_WIDTH_MAX  1024
#define GUI_WINDOW_HEIGHT_MIN 1
#define GUI_WINDOW_HEIGHT_MAX 1024

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    rect_t _dim;
} gui_window_t;

/* --- Public Functions --- */
gui_ret_t GUI_Window_New(gui_window_t* this, int32_t posx, int32_t posy, bool visible, int32_t width, int32_t height);

void (*GUI_Window_GetDestructor(void))(void*);

gui_ret_t GUI_Window_SetPosition(gui_window_t* this, int32_t posx, int32_t posy);

point_t GUI_Window_GetPosition(gui_window_t* this);

gui_ret_t GUI_Window_SetDimensions(gui_window_t* this, int32_t width, int32_t height);

rect_t GUI_Window_GetDimensions(gui_window_t* this);
#endif
