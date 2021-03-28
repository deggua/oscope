#include "gui/elements/gui_graph.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"
#include "utils/scope.h"

static void Destructor(void* this) {}

static void Render(void* this, gui_theme_t* theme, screen_t* scr, point_t origin) {}

void (*GUI_Graph_GetDestructor(void))(void*) {
    return &Destructor;
}

gui_ret_t GUI_Graph_New(gui_graph_t* this, int32_t posx, int32_t posy, bool visible, int32_t width, int32_t height) {
    gui_ret_t ret;

    // initialize the base class
    ret = GUI_Object_New((gui_object_t*)this, posx, posy, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // initialize the graph class
    ret = GUI_Graph_SetDimensions(this, width, height);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    this->_cursors = LinkedList_New();

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;

    return ret;
}

gui_ret_t GUI_Graph_SetPosition(gui_graph_t* this, int32_t posx, int32_t posy) {
    return GUI_Object_SetPosition((gui_object_t*)this, posx, posy);
}

point_t GUI_Graph_GetPosition(gui_graph_t* this) {
    return GUI_Object_GetPosition((gui_object_t*)this);
}

gui_ret_t GUI_Graph_SetDimensions(gui_graph_t* this, int32_t width, int32_t height) {
    if (width < GUI_GRAPH_WIDTH_MIN || width > GUI_GRAPH_WIDTH_MAX || height < GUI_GRAPH_HEIGHT_MIN ||
        height > GUI_GRAPH_HEIGHT_MAX) {
        return GUI_RET_FAILURE_INVALID;
    }

    this->_dim.w = width;
    this->_dim.h = height;

    return GUI_RET_SUCCESS;
}

rect_t GUI_Graph_GetDimensions(gui_graph_t* this) {
    return this->_dim;
}

gui_waveform_t* GUI_Graph_GetWaveform(gui_graph_t* this, scope_channel_t channel) {
    return &this->_waveforms[channel];
}

gui_cursor_index_t GUI_Graph_AddCursor(float time) {}

void GUI_Graph_SetCursor(gui_cursor_index_t cursor, float time) {}

gui_ret_t GUI_Graph_RemoveCursor(gui_cursor_index_t cursor) {}
