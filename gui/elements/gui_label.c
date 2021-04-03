#include "gui/elements/gui_label.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"


/* --- Private Functions --- */
static void Destructor(void* this) {
    gui_label_t* thisLabel = this;
    free(thisLabel->_text);

    // call base class destructor
    GUI_Object_GetDestructor()((gui_object_t*)thisLabel);
    return;
}

static void Render(void* this, gui_theme_t* theme, point_t origin) {
    gui_label_t* thisLabel = this;
    point_t      posLabel   = GUI_Label_GetPosition(thisLabel);
    int32_t      scaleLabel = GUI_Label_GetScale(thisLabel);
    const char*  textLabel  = GUI_Label_GetText(thisLabel);

    SCR_DrawString(origin.x + posLabel.x, origin.y + posLabel.y, textLabel, scaleLabel, theme->text);

    return;
}

/* --- Public Functions --- */
gui_ret_t GUI_Label_New(gui_label_t* this, int32_t posx, int32_t posy, bool visible, char* text, int32_t scale) {
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

    ret = GUI_Label_SetScale(this, scale);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;

    return ret;
}

void (*GUI_Label_GetDestructor(void))(void*) {
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

gui_ret_t GUI_Label_SetScale(gui_label_t* this, int32_t scale) {
    if (scale < 1) {
        return GUI_RET_FAILURE_INVALID;
    }

    this->_scale = scale;
    return GUI_RET_SUCCESS;
}

int32_t GUI_Label_GetScale(gui_label_t* this) {
    return this->_scale;
}
