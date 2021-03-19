#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui_base.h"
#include "gui/elements/gui_button.h"

static void Destructor(gui_button_t* button) {
    free(button->_text);

    // class base class destructor
    GUI_Object_GetDestructor()((gui_object_t*)button);
    return;
}

void (* GUI_Button_GetDestructor(void))(gui_button_t*) {
    return &Destructor;
}

// Validates a button's position
// Returns true if (width,height) are within the bounds set in the #defines
static bool GUI_ValidateButtonDimensions(
    const int32_t width, const int32_t height) {

}

gui_ret_t GUI_Button_New(
    gui_button_t* this, 
    int32_t posx, int32_t posy, 
    int32_t width, int32_t height, 
    const char* text, bool visible) {
    
    // initialize the base class
    GUI_Object_New((gui_object_t*)this, posx, posy, visible);

    // initialize the button class
    gui_ret_t ret;
    ret = GUI_Button_SetDimensions(this, width, height);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    
    ret = GUI_Button_SetText(this, text);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;

    return;
}

// Validates and sets the position of a button
gui_ret_t SetPosition(gui_button_t* this, const int32_t posx, const int32_t posy) {
    if (!GUI_ValidateElementPosition(posx, posy)) {
        return GUI_RET_FAILURE_INVALID;
    }

    button->_pos.x = posx;
    button->_pos.y = posy;

    return GUI_RET_SUCCESS;
}

// Validates and sets the dimensions of a button
gui_ret_t SetDimensions(gui_button_t* button, const int32_t width, const int32_t height) {
    if (button == NULL) {
        return GUI_RET_FAILURE_NULLPTR;
    }

    if (!GUI_ValidateButtonDimensions(width, height)) {
        return GUI_RET_FAILURE_INVALID;
    }

    button->_dim.w = width;
    button->_dim.h = height;

    return GUI_RET_SUCCESS;
}

// Changes the text displayed on a button
gui_ret_t SetText(gui_button_t* button, const char* text) {
    if (button == NULL) {
        return GUI_RET_FAILURE_NULLPTR;
    }

    if (text == NULL) {
        free(button->_text);
        button->_text = NULL;
    } else {
        char* strNewText = malloc(strlen(text));
        if (strNewText == NULL) {
            return GUI_RET_FAILURE_NOMEM;
        }

        strcpy(strNewText, text);
        free(button->_text);
        button->_text = strNewText;
    }

    return GUI_RET_SUCCESS;
}
