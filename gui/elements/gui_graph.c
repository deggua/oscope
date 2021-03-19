#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui_base.h"
#include "gui/elements/gui_graph.h"

static void GUI_DestructorGraph(void* element) {
    gui_graph_t* graph = (gui_graph_t*)element;
    // !!! FILL WITH DESTRUCTOR CODE FOR WAVEFORMS !!! //
    free(graph);
    return;
}