#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "scope_gui.h"

static void GUI_DestructorButton(void* element) {
    gui_button_t* button = (gui_button_t*)element;
    free(button->_text);
    free(button);
    return;
}

static void GUI_DestructorLabel(void* element) {
    gui_label_t* label = (gui_label_t*)element;
    free(label->_text);
    free(label);
    return;
}

static void GUI_DestructorGraph(void* element) {
    gui_graph_t* graph = (gui_graph_t*)element;
    // !!! FILL WITH DESTRUCTOR CODE FOR WAVEFORMS !!! //
    free(graph);
    return;
}

static void GUI_DestructorWindow(void* element) {
    gui_window_t* window = (gui_window_t*)element;
    free(window);
    return;
}

// Allocates and creates the object and element for a specified type of GUI element
gui_object_t* GUI_CreateObject(gui_element_t type) {
    size_t szObject;
    void (* callbackDestructor)(void*);

    switch (type)
    {
    case GUI_ELEMENT_WINDOW:
        szObject = sizeof(gui_window_t);
        callbackDestructor = &GUI_DestructorWindow;
        break;
    case GUI_ELEMENT_BUTTON:
        szObject = sizeof(gui_button_t);
        callbackDestructor = &GUI_DestructorButton;
        break;
    case GUI_ELEMENT_LABEL:
        szObject = sizeof(gui_label_t);
        callbackDestructor = &GUI_DestructorLabel;
        break;
    case GUI_ELEMENT_GRAPH:
        szObject = sizeof(gui_graph_t);
        callbackDestructor = &GUI_DestructorGraph;
        break;
    default:
        //invalid type
        return NULL;
    }

    //allocate and initialize the object container and its element
    gui_object_t* object  = calloc(1, sizeof(gui_object_t));
    void* element         = calloc(1, szObject);
    if (object == NULL || element == NULL) { return NULL; }

    object->_elem = element;
    object->_type = type;
    object->_destructor = callbackDestructor;
    object->visible = false;

    return object;
}

// Adds an existing GUI object to a parent object
gui_ret_t GUI_AddObject(gui_object_t* obj, gui_object_t* parent) {

    //allocate or extend the buffer for children if necessary
    if (parent->_children == NULL) {
        gui_object_t** bufChildren = calloc(GUI_CHILDREN_INIT_ALLOC, sizeof(gui_object_t*));
        if (bufChildren == NULL) { return NULL; }

        parent->_num_children_max = GUI_CHILDREN_INIT_ALLOC;
        parent->_children = bufChildren;

    } else if (parent->_num_children_cur == parent->_num_children_max) {
        gui_object_t** bufChildren = realloc(parent->_children, 2 * parent->_num_children_max);
        if (bufChildren == NULL) { return NULL; }

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
            parent->_children[ii] = obj;
            obj->_parent = parent;
            return GUI_RET_SUCCESS;
        }
    }
    
    return GUI_RET_ERROR;
}

// Removes an exisitng GUI object from its parent object
gui_ret_t GUI_RemoveObject(gui_object_t* obj) {
    //if the object was null the operation is invalid
    if (obj == NULL) {
        return GUI_RET_FAILURE_INVALID;
    }

    //parent could be null, in which case we don't do anything
    gui_object_t* parent = obj->_parent;
    if (parent == NULL) {
        return GUI_RET_SUCCESS;
    }

    //detach from parent
    for (int32_t ii = 0; ii < parent->_num_children_max; ii++) {
        if (parent->_children[ii] == obj) {
            parent->_children[ii] = NULL;
            parent->_num_children_cur--;
            obj->_parent = NULL;
            return GUI_RET_SUCCESS;
        }
    }

    //if it gets here, it means the child pointed to the parent, but the parent didn't have the child listed
    //which means the GUI was constructed improperly and that bad things probably happened
    return GUI_RET_ERROR;
}

// Destroys an exisitng GUI object and all its children
gui_ret_t GUI_DestroyObject(gui_object_t* obj) {
    //don't need to do anything if it doesn't exist
    if (obj == NULL) {
        return GUI_RET_SUCCESS;
    }

    //destroy children
    for (int32_t ii = 0; ii < obj->_num_children_max; ii++) {
        if (obj->_children[ii] != NULL) {
            GUI_DestroyObject(obj->_children[ii]);
        }
    }

    //free memory
    obj->_destructor(obj->_elem);
    free(obj->_children);
    free(obj);
    return GUI_RET_SUCCESS;
}