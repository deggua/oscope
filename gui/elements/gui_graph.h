#ifndef GUI_GRAPH_H
#define GUI_GRAPH_H

#include <stdint.h>

#include "gui/gui_base.h"
#include "utils/containers.h"
#include "utils/geometry.h"
#include "utils/numeric.h"
#include "utils/scope.h"

#define GUI_GRAPH_NUM_WAVEFORMS 2UL

#define GUI_GRAPH_WIDTH_MIN  1UL
#define GUI_GRAPH_WIDTH_MAX  1024UL
#define GUI_GRAPH_HEIGHT_MIN 1UL
#define GUI_GRAPH_HEIGHT_MAX 1024UL

#define GUI_WAVEFORM_NUM_POINTS 64UL

typedef size_t gui_cursor_index_t;

typedef struct {
    range_t x, y;

    float samples[GUI_WAVEFORM_NUM_POINTS];
} gui_waveform_t;

typedef struct {
    float pos;
} gui_cursor_t;

typedef struct {
    gui_object_t _base;

    rect_t _dim;

    gui_waveform_t _waveforms[GUI_GRAPH_NUM_WAVEFORMS];
    linkedlist_t*  _cursors;
} gui_graph_t;

void (*GUI_Graph_GetDestructor(void))(void*);

gui_ret_t GUI_Graph_New(gui_graph_t* this, int32_t posx, int32_t posy, bool visible, int32_t width, int32_t height);

gui_ret_t GUI_Graph_SetPosition(gui_graph_t* this, int32_t posx, int32_t posy);

point_t GUI_Graph_GetPosition(gui_graph_t* this);

gui_ret_t GUI_Graph_SetDimensions(gui_graph_t* this, int32_t width, int32_t height);

rect_t GUI_Graph_GetDimensions(gui_graph_t* this);

gui_waveform_t* GUI_Graph_GetWaveform(gui_graph_t* this, scope_channel_t channel);

gui_cursor_index_t GUI_Graph_AddCursor(float time);

void GUI_Graph_SetCursor(gui_cursor_index_t cursor, float time);

gui_ret_t GUI_Graph_RemoveCursor(gui_cursor_index_t cursor);

#endif
