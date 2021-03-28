#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include <stdint.h>

#include "gui/gui_base.h"
#include "utils/geometry.h"

#define GUI_BUTTON_WIDTH_MIN  1
#define GUI_BUTTON_WIDTH_MAX  1024
#define GUI_BUTTON_HEIGHT_MIN 1
#define GUI_BUTTON_HEIGHT_MAX 1024

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    rect_t _dim;
    char*  _text;
} gui_button_t;

/* --- Public Functions --- */
// Return the destructor function for a button
void (*GUI_Button_GetDestructor(void))(void*);

// Create a new button
gui_ret_t GUI_Button_New(
    gui_button_t* this,
    int32_t     posx,
    int32_t     posy,
    bool        visible,
    int32_t     width,
    int32_t     height,
    const char* text);

// Set the position of a button
gui_ret_t GUI_Button_SetPositon(gui_button_t* this, int32_t posx, int32_t posy);

// Get the position of a button
point_t GUI_Button_GetPositon(gui_button_t* this);

// Sets the dimensions of a button
gui_ret_t GUI_Button_SetDimensions(gui_button_t* this, int32_t width, int32_t height);

// Gets the dimensions of a button
rect_t GUI_Button_GetDimensions(gui_button_t* this);

// Sets the text displayed on a button
gui_ret_t GUI_Button_SetText(gui_button_t* this, const char* text);

// Returns the text displayed on a button
const char* GUI_Button_GetText(gui_button_t* this);

#endif
