#ifndef GUI_GRAPH_H
#define GUI_GRAPH_H

#include <stdint.h>

#include "utils/geometry.h"
#include "gui/gui_base.h"

#define GUI_MAX_WAVEFORMS 2UL

typedef struct {
    float xlower, xupper;
    float ylower, yupper;

    float* samples;
    size_t num_samples;
} gui_graph_waveform_t;

typedef struct {


    rect_t dim;

    float xlower, xupper;
    float ylower, yupper;

    gui_graph_waveform_t* _waveforms[GUI_MAX_WAVEFORMS];
} gui_graph_t;

#endif