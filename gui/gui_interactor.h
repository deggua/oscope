#ifndef GUI_INTERACTOR_H
#define GUI_INTERACTOR_H

#include "gui/gui_base.h"

typedef struct {
    gui_interactor_t *up, *down, *left, *right;
    gui_object_t*     elem;
} gui_interactor_t;

#endif
