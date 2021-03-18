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

gui_object_t* GUI_AddElement(gui_object_t* parent, gui_type_t type) {

    size_t szObject;
    void (* callbackDestructor)(void*);

    switch (type)
    {
    case GUI_WINDOW:
        szObject = sizeof(gui_window_t);
        callbackDestructor = &GUI_DestructorWindow;
        break;
    case GUI_BUTTON:
        szObject = sizeof(gui_button_t);
        callbackDestructor = &GUI_DestructorButton;
        break;
    case GUI_LABEL:
        szObject = sizeof(gui_label_t);
        callbackDestructor = &GUI_DestructorLabel;
        break;
    case GUI_GRAPH:
        szObject = sizeof(gui_graph_t);
        callbackDestructor = &GUI_DestructorGraph;
        break;
    default:
        //invalid type
        return NULL;
    }

    //allocate and initialize the container and its element for the new element
    gui_object_t* container  = calloc(1, sizeof(gui_object_t));
    void* element            = calloc(1, szObject);
    if (container == NULL || element == NULL) { return NULL; }

    container->_elem = element;
    container->_type = type;
    container->_parent = parent;
    container->_destructor = callbackDestructor;
    container->visible = false;

    //allocate and add the element as a child of the parent
    if (parent->_children == NULL) {
        gui_object_t** bufChildren = calloc(GUI_CHILDREN_INIT_ALLOC, sizeof(gui_object_t*));
        if (bufChildren == NULL) { 
            //free the element since we couldn't allocate space for it as a child
            free(container);
            free(element);
            return NULL; 
        }

        memset(&bufChildren[0], 0x00, GUI_CHILDREN_INIT_ALLOC * sizeof(gui_object_t*));
        container->_num_children_max = GUI_CHILDREN_INIT_ALLOC;
        container->_children = bufChildren;

    } else if (parent->_num_children_cur == parent->_num_children_max) {
        gui_object_t** bufChildren = realloc(parent->_children, 2 * parent->_num_children_max);
        if (bufChildren == NULL) { 
            //free the element since we couldn't allocate space for it as a child
            free(container);
            free(element);
            return NULL; 
        }

        memset(
            &bufChildren[parent->_num_children_max], 
            0x00, 
            parent->_num_children_max * sizeof(gui_object_t*)
        );

        container->_num_children_max = 2 * container->_num_children_max;
        container->_children = bufChildren;
    }

    //find the first open child slot
    for (int32_t ii = 0; ii < container->_num_children_max; ii++) {
        if (parent->_children[ii] != NULL) {
            parent->_children[ii] = container;
            return container;
        }
    }
    
    //failed to find a slot for some reason, this shouldn't be possible, but we can handle it anyway
    free(container);
    free(element);
    return NULL;
}

void GUI_DeleteElement(gui_object_t* obj) {
    //don't need to do anything if it's already deleted
    if (obj == NULL) {
        return;
    }

    //detach from parent
    gui_object_t* parent = obj->_parent;
    for (int32_t ii = 0; ii < parent->_num_children_max; ii++) {
        if (parent->_children[ii] == obj) {
            parent->_children[ii] == NULL;
            parent->_num_children_cur--;
            break;
        }
    }

    //delete children
    for (int32_t ii = 0; ii < obj->_num_children_max; ii++) {
        if (obj->_children[ii] != NULL) {
            GUI_DeleteElement(obj->_children[ii]);
        }
    }

    //free memory
    obj->_destructor(obj->_elem);
    free(obj->_children);
    free(obj);
    return;
}