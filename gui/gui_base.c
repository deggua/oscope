#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils/class.h"
#include "gui/gui.h"
#include "utils/geometry.h"

#include "gui/gui_base.h"

/* --- Private Functions --- */
static void Destructor(gui_object_t* this) {
    //destroy children
    for (int32_t ii = 0; ii < this->_num_children_max; ii++) {
        if (this->_children[ii] != NULL) {
            Destructor(this->_children[ii]);
        }
    }

    //free memory
    free(this->_children);

    //call superclass destructor
    Class_GetDestructor()((class_t*)this);
    return GUI_RET_SUCCESS;
}

static void Render(gui_object_t* this, gui_theme_t* theme, screen_t* scr, point_t origin) {
    //abstract method, needs to be implemented by the derived class
    return;
}

/* --- Public Functions --- */
void (* GUI_Object_GetDestructor(void))(gui_object_t*) {
    return &Destructor;
}

gui_ret_t GUI_Object_New(gui_object_t* this, int32_t posx, int32_t posy, bool visible) {
    gui_ret_t ret;

    //call superclass constructor
    Class_New((class_t*)this);

    //setup member variables
    this->_parent = NULL;
    this->_children = NULL;
    this->_num_children_max = 0;
    this->_num_children_cur = 0;

    ret = GUI_Object_SetVisiblity(this, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    ret = GUI_Object_SetPosition(this, posx, posy);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    //setup member functions
    this->_Render = &Render;

    //replace destructor
    this->_base._Destructor = &Destructor;
    return GUI_RET_SUCCESS;
}

// Adds an existing GUI object to a parent object
gui_ret_t GUI_Object_Add(gui_object_t* this, gui_object_t* parent) {

    //allocate or extend the buffer for children if necessary
    if (parent->_children == NULL) {
        gui_object_t** bufChildren = calloc(GUI_CHILDREN_INIT_ALLOC, sizeof(gui_object_t*));
        if (bufChildren == NULL) {
             return GUI_RET_FAILURE_NOMEM; 
        }

        parent->_num_children_max = GUI_CHILDREN_INIT_ALLOC;
        parent->_children = bufChildren;

    } else if (parent->_num_children_cur == parent->_num_children_max) {
        gui_object_t** bufChildren = realloc(parent->_children, 2 * parent->_num_children_max);
        if (bufChildren == NULL) {
            return GUI_RET_FAILURE_NOMEM;
        }

        memset(
            &bufChildren[parent->_num_children_max], 
            0x00, 
            parent->_num_children_max * sizeof(gui_object_t*)
        );

        parent->_num_children_max = 2 * parent->_num_children_max;
        parent->_children = bufChildren;
    }

    //find the first open child slot
    for (int32_t ii = 0; ii < parent->_num_children_max; ii++) {
        if (parent->_children[ii] != NULL) {
            parent->_children[ii] = this;
            this->_parent = parent;
            return GUI_RET_SUCCESS;
        }
    }
    
    return GUI_RET_ERROR;
}

// Removes an exisitng GUI object from its parent object
gui_ret_t GUI_Object_Remove(gui_object_t* this) {
    //if the object was null the operation is invalid
    if (this == NULL) {
        return GUI_RET_FAILURE_INVALID;
    }

    //parent could be null, in which case we don't do anything
    gui_object_t* parent = this->_parent;
    if (parent == NULL) {
        return GUI_RET_SUCCESS;
    }

    //detach from parent
    for (int32_t ii = 0; ii < parent->_num_children_max; ii++) {
        if (parent->_children[ii] == this) {
            parent->_children[ii] = NULL;
            parent->_num_children_cur--;
            this->_parent = NULL;
            return GUI_RET_SUCCESS;
        }
    }

    //if it gets here, it means the child pointed to the parent, but the parent didn't have the child listed
    //which means the GUI was constructed improperly and that bad things probably happened
    return GUI_RET_ERROR;
}

gui_ret_t GUI_Object_SetPosition(gui_object_t* this, int32_t x, int32_t y) {
    this->_pos.x = x;
    this->_pos.y = y;

    return GUI_RET_SUCCESS;
}

point_t GUI_Object_GetPosition(gui_object_t* this) {
    return this->_pos;
}

gui_ret_t GUI_Object_SetVisiblity(gui_object_t* this, bool visible) {
    this->_visible = visible;

    return GUI_RET_SUCCESS;
}

bool GUI_Object_GetVisibility(gui_object_t* this) {
    return this->_visible;
}

void GUI_Object_Render(gui_object_t* this, gui_theme_t* theme, screen_t* scr, point_t origin) {
    this->_Render(this, theme, scr, origin);

    for (int32_t ii = 0; ii < this->_num_children_max; ii++) {
        if (this->_children[ii] != NULL) {
            gui_object_t* child = this->_children[ii];
            origin.x += this->_pos.x;
            origin.y += this->_pos.y;
            GUI_Object_Render(child, theme, scr, origin);
        }
    }

    return;
}