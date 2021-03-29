#include "gui/gui_interactor.h"

#include <stdbool.h>

#include "drivers/user_controls.h"

gui_interactor_t* GUI_Interactor_Select(gui_interactor_t* this, joy_dir_t dir) {
    if (dir == JOY_DIR_NONE) {
        this->elem->_sel = true;
        return this;
    } else if (dir == JOY_DIR_UP && this->up != NULL) {
        this->elem->_sel     = false;
        this->up->elem->_sel = true;
        return this->up;
    } else if (dir == JOY_DIR_DOWN && this->down != NULL) {
        this->elem->_sel       = false;
        this->down->elem->_sel = true;
        return this->down;
    } else if (dir == JOY_DIR_RIGHT && this->right != NULL) {
        this->elem->_sel        = false;
        this->right->elem->_sel = true;
        return this->right;
    } else if (dir == JOY_DIR_LEFT && this->left != NULL) {
        this->elem->_sel       = false;
        this->left->elem->_sel = true;
        return this->left;
    } else {
        this->elem->_sel = true;
        return this;
    }
}

void GUI_Interactor_Execute(gui_interactor_t* this) {
    this->callback();
    return;
}
