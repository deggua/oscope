#ifndef GUI_GRAPH_H
#define GUI_GRAPH_H

#include <stdint.h>

#include "utils/geometry.h"
#include "gui/gui_base.h"
#include "utils/numeric.h"
#include "utils/containers.h"

#define GUI_GRAPH_NUM_WAVEFORMS 2UL

#define GUI_GRAPH_WIDTH_MIN 1UL
#define GUI_GRAPH_WIDTH_MAX 1024UL
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
    linkedlist_t* _cursors;
} gui_graph_t;

#endif