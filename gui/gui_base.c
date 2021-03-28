#include "gui/gui_base.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui.h"
#include "utils/class.h"
#include "utils/geometry.h"

/* --- Private Functions --- */
static void Destructor(void* this) {
    // destroy children
	gui_object_t* thisObject = this;
    linkedlist_t* child = LinkedList_HeadOf(thisObject->_children);
    while (child != NULL) {
        Destructor(child->val);
        child = child->next;
    }

    // free memory
    LinkedList_DeleteList(thisObject->_children);

    // call superclass destructor
    Class_GetDestructor()((class_t*)thisObject);
    return;
}

static void Render(void* this, gui_theme_t* theme, screen_t* scr, point_t origin) {
    // abstract method, needs to be implemented by the derived class
    return;
}

/* --- Public Functions --- */
void (*GUI_Object_GetDestructor(void))(void*) {
    return &Destructor;
}

gui_ret_t GUI_Object_New(gui_object_t* this, int32_t posx, int32_t posy, bool visible) {
    gui_ret_t ret;

    // call superclass constructor
    Class_New((class_t*)this);

    // setup member variables
    this->_parent   = NULL;
    this->_children = NULL;

    ret = GUI_Object_SetVisiblity(this, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    ret = GUI_Object_SetPosition(this, posx, posy);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // setup member functions
    this->_Render = &Render;

    // replace destructor
    ((class_t*)this)->_Destructor = &Destructor;
    return GUI_RET_SUCCESS;
}

// Adds an existing GUI object to a parent object
gui_ret_t GUI_Object_Add(gui_object_t* this, gui_object_t* parent) {
    // create the new child node for storage
    linkedlist_t* child = LinkedList_New();
    if (child == NULL) {
        return GUI_RET_FAILURE_NOMEM;
    }
    child->val = this;

    // either set the buffer to the child, or append it
    if (parent->_children == NULL) {
        parent->_children = child;
    } else {
        parent->_children = LinkedList_AppendNode(parent->_children, child);
    }

    return GUI_RET_SUCCESS;
}

// Removes an exisitng GUI object from its parent object
gui_ret_t GUI_Object_Remove(gui_object_t* this) {
    // if the object was null the operation is invalid
    if (this == NULL) {
        return GUI_RET_FAILURE_INVALID;
    }

    // parent could be null, in which case we don't do anything
    gui_object_t* parent = this->_parent;
    if (parent == NULL) {
        return GUI_RET_SUCCESS;
    }

    // find child and detach from parent
    linkedlist_t* child = LinkedList_HeadOf(parent->_children);
    while (child != NULL) {
        if (child->val == this) {
            parent->_children = LinkedList_RemoveNode(child);
            return GUI_RET_SUCCESS;
        }
        child = child->next;
    }

    // if we got here the parent was malformed, which means something bad
    // happened earlier
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

    linkedlist_t* child = LinkedList_HeadOf(this->_children);
    while (child != NULL) {
        point_t child_origin;
        child_origin.x = origin.x + this->_pos.x;
        child_origin.y = origin.y + this->_pos.y;

        GUI_Object_Render(child->val, theme, scr, child_origin);
    }

    return;
}
