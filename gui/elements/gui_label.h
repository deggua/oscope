#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include <stdint.h>

#include "utils/geometry.h"
#include "gui/gui_base.h"

typedef struct {
    // derived from
    gui_object_t _base;

    // member variables
    char* _text;
} gui_label_t;

#endif