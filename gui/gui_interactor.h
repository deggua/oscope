#ifndef GUI_INTERACTOR_H
#define GUI_INTERACTOR_H

#include "gui/elements/gui_button.h"
#include "gui/gui_base.h"
#include "drivers/user_controls.h"

typedef struct GUI_INTERACTOR_T {
    struct GUI_INTERACTOR_T *up, *down, *left, *right;
    gui_button_t*            elem;
    void (*callback)(void);
} gui_interactor_t;

gui_interactor_t* GUI_Interactor_Select(gui_interactor_t* this, joy_dir_t dir);

void GUI_Interactor_Execute(gui_interactor_t* this);

#endif
