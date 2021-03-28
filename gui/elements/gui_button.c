#include "gui/elements/gui_button.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"
#include "gui/gui_renderer.h"
#include "utils/geometry.h"

/* --- Private Functions --- */
static void Destructor(void* this) {
    gui_button_t* thisButton = this;
    free(thisButton->_text);

    // call base class destructor
    GUI_Object_GetDestructor()((gui_object_t*)thisButton);
    return;
}

static void Render(void* this, gui_theme_t* theme, point_t origin) {
    gui_button_t* thisButton;
    point_t       posButton  = GUI_Button_GetPosition(thisButton);
    rect_t        dimButton  = GUI_Button_GetDimensions(thisButton);
    const char*   textButton = GUI_Button_GetText(thisButton);

    SCR_DrawRectangle(
        origin.x + posButton.x,
        origin.y + posButton.y,
        origin.x + posButton.x + dimButton.w,
        origin.y + posButton.y + dimButton.h,
        false,
        theme->border);

    SCR_DrawString(origin.x + posButton.x, origin.y + posButton.y, textButton, 1, theme->text);

    return;
}

/* --- Public Functions --- */
// Return the destructor function for a button
void (*GUI_Button_GetDestructor(void))(void*) {
    return &Destructor;
}

// Create a new button
gui_ret_t GUI_Button_New(
    gui_button_t* this,
    int32_t     posx,
    int32_t     posy,
    bool        visible,
    int32_t     width,
    int32_t     height,
    const char* text) {
    gui_ret_t ret;

    // initialize the base class
    ret = GUI_Object_New((gui_object_t*)this, posx, posy, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // initialize the button class
    ret = GUI_Button_SetDimensions(this, width, height);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    ret = GUI_Button_SetText(this, text);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;
    return ret;
}

// Set the position of a button
gui_ret_t GUI_Button_SetPosition(gui_button_t* this, int32_t posx, int32_t posy) {
    return GUI_Object_SetPosition((gui_object_t*)this, posx, posy);
}

// Get the position of a button
point_t GUI_Button_GetPosition(gui_button_t* this) {
    return GUI_Object_GetPosition((gui_object_t*)this);
}

// Sets the dimensions of a button
gui_ret_t GUI_Button_SetDimensions(gui_button_t* this, int32_t width, int32_t height) {
    if (width < GUI_BUTTON_WIDTH_MIN || width > GUI_BUTTON_WIDTH_MAX || height < GUI_BUTTON_HEIGHT_MIN ||
        height > GUI_BUTTON_HEIGHT_MAX) {
        return GUI_RET_FAILURE_INVALID;
    }

    this->_dim.w = width;
    this->_dim.h = height;

    return GUI_RET_SUCCESS;
}

// Gets the dimensions of a button
rect_t GUI_Button_GetDimensions(gui_button_t* this) {
    return this->_dim;
}

// Sets the text displayed on a button
gui_ret_t GUI_Button_SetText(gui_button_t* this, const char* text) {
    if (text == NULL) {
        free(this->_text);
        this->_text = NULL;
    } else {
        char* strNewText = malloc(strlen(text));
        if (strNewText == NULL) {
            return GUI_RET_FAILURE_NOMEM;
        }

        strcpy(strNewText, text);
        free(this->_text);
        this->_text = strNewText;
    }

    return GUI_RET_SUCCESS;
}

// Returns the text displayed on a button
const char* GUI_Button_GetText(gui_button_t* this) {
    return this->_text;
}
