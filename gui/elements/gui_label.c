#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui_base.h"
#include "gui/elements/gui_label.h"

/* --- Private Functions --- */
static void Destructor(gui_label_t* this) {

    free(this->_text);

    // call base class destructor
    GUI_Object_GetDestructor()((gui_object_t*)this);
    return;
}

static void Render(gui_label_t* this, gui_theme_t* theme, screen_t* scr, point_t origin) {
    
}

/* --- Public Functions --- */
gui_ret_t GUI_Label_New(
    gui_label_t* this, 
    int32_t posx, int32_t posy, bool visible, 
    char* text) {

    gui_ret_t ret;

    // initialize the base class
    ret = GUI_Object_New((gui_object_t*)this, posx, posy, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // initialize the label class
    ret = GUI_Label_SetText(this, text);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;

    return ret;
}

void (* GUI_Label_GetDestructor(void))(gui_label_t*) {
    return &Destructor;
}

gui_ret_t GUI_Label_SetPosition(gui_label_t* this, int32_t posx, int32_t posy) {
    return GUI_Object_SetPosition((gui_object_t*)this, posx, posy);
}

point_t GUI_Label_GetPosition(gui_label_t* this) {
    return GUI_Object_GetPosition((gui_object_t*)this);
}

gui_ret_t GUI_Label_SetText(gui_label_t* this, const char* text) {
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

const char* GUI_Label_GetText(gui_label_t* this) {
    return this->_text;
}
