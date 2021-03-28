#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include <stdint.h>

#include "gui/gui_base.h"
#include "utils/geometry.h"

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    char*   _text;
    int32_t _scale;
} gui_label_t;

/* --- Public Functions --- */
gui_ret_t GUI_Label_New(gui_label_t* this, int32_t posx, int32_t posy, bool visible, char* text, int32_t scale);

void (*GUI_Label_GetDestructor(void))(void*);

gui_ret_t GUI_Label_SetPosition(gui_label_t* this, int32_t posx, int32_t posy);

point_t GUI_Label_GetPosition(gui_label_t* this);

gui_ret_t GUI_Label_SetText(gui_label_t* this, const char* text);

const char* GUI_Label_GetText(gui_label_t* this);

#endif
