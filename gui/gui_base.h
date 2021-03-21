#ifndef GUI_BASE_H
#define GUI_BASE_H

#include <stdint.h>

#include "utils/class.h"
#include "utils/geometry.h"
#include "gui/gui.h"
#include "gui/gui_renderer.h"
#include "display/display.h"

#define GUI_CHILDREN_INIT_ALLOC 1UL

typedef struct GUI_OBJECT {
    // derived from
    class_t _base;

    // member variables
    struct GUI_OBJECT* _parent;
    struct GUI_OBJECT** _children;
    size_t _num_children_cur;
    size_t _num_children_max;

    bool _visible;
    point_t _pos;

    // override functions
    void (* _Render)(void*, gui_theme_t*, screen_t*, point_t);
} gui_object_t;

void (* GUI_Object_GetDestructor(void))(gui_object_t*);

gui_ret_t GUI_Object_New(gui_object_t* this, int32_t posx, int32_t posy, bool visible);

// Adds an existing GUI object to a parent object
gui_ret_t GUI_Object_Add(gui_object_t* this, gui_object_t* parent);

// Removes an exisitng GUI object from its parent object
gui_ret_t GUI_Object_Remove(gui_object_t* this);

gui_ret_t GUI_Object_SetPosition(gui_object_t* this, int32_t x, int32_t y);

point_t GUI_Object_GetPosition(gui_object_t* this);

gui_ret_t GUI_Object_SetVisiblity(gui_object_t* this, bool visible);

bool GUI_Object_GetVisibility(gui_object_t* this);

#endif