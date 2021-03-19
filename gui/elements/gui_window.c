#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui_base.h"
#include "gui/elements/gui_window.h"

static void GUI_DestructorWindow(void* element) {
    gui_window_t* window = (gui_window_t*)element;
    free(window);
    return;
}