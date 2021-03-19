#include <stdint.h>

#include "utils/geometry.h"

#define GUI_MAX_WAVEFORMS 2UL

typedef struct {
    float xlower, xupper;
    float ylower, yupper;

    float* samples;
    size_t num_samples;
} graph_waveform_t;

typedef struct {
    rect_t dim;

    float xlower, xupper;
    float ylower, yupper;

    graph_waveform_t* _waveforms[GUI_MAX_WAVEFORMS];
} gui_graph_t;
