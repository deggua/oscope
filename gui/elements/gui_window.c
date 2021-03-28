#include "gui/elements/gui_window.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"
#include "utils/geometry.h"

/* --- Private Functions --- */
static void Destructor(void* this) {
    // call base class destructor
    GUI_Object_GetDestructor()((gui_object_t*)this);
    return;
}

static void Render(void* this, gui_theme_t* theme, point_t origin) {
    gui_window_t* thisWindow = (gui_window_t*)this;
    point_t       posWindow  = GUI_Window_GetPosition(thisWindow);
    rect_t        dimWindow  = GUI_Window_GetDimensions(thisWindow);

    SCR_DrawRectangle(
        origin.x + posWindow.x,
        origin.y + posWindow.y,
        origin.x + posWindow.x + dimWindow.w,
        origin.y + posWindow.y + dimWindow.h,
        true,
        theme->background);

    SCR_DrawRectangle(
        origin.x + posWindow.x,
        origin.y + posWindow.y,
        origin.x + posWindow.x + dimWindow.w,
        origin.y + posWindow.y + dimWindow.h,
        false,
        theme->border);

    return;
}

/* --- Public Functions --- */
gui_ret_t GUI_Window_New(gui_window_t* this, int32_t posx, int32_t posy, bool visible, int32_t width, int32_t height) {
    gui_ret_t ret;

    // initialize base class
    ret = GUI_Object_New((gui_object_t*)this, posx, posy, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // initialize the window class
    ret = GUI_Window_SetDimensions(this, width, height);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;
    return ret;
}

void (*GUI_Window_GetDestructor(void))(void*) {
    return &Destructor;
}

gui_ret_t GUI_Window_SetPosition(gui_window_t* this, int32_t posx, int32_t posy) {
    return GUI_Object_SetPosition((gui_object_t*)this, posx, posy);
}

point_t GUI_Window_GetPosition(gui_window_t* this) {
    return GUI_Object_GetPosition((gui_object_t*)this);
}

gui_ret_t GUI_Window_SetDimensions(gui_window_t* this, int32_t width, int32_t height) {
    if (width < GUI_WINDOW_WIDTH_MIN || width > GUI_WINDOW_WIDTH_MAX || height < GUI_WINDOW_HEIGHT_MIN ||
        height > GUI_WINDOW_HEIGHT_MAX) {
        return GUI_RET_FAILURE_INVALID;
    }

    this->_dim.w = width;
    this->_dim.h = height;

    return GUI_RET_SUCCESS;
}

rect_t GUI_Window_GetDimensions(gui_window_t* this) {
    return this->_dim;
}
