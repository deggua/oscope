#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/gui_base.h"
#include "gui/elements/gui_label.h"

/* -- Public Functions --- */
gui_object_t* GUI_CreateLabel(const int32_t posx, const int32_t posy, const char* text) {
    if (!GUI_ValidateElementPosition(posx, posy)) {
        return GUI_RET_FAILURE_INVALID;
    }

    gui_object_t* objLabel = GUI_CreateObject();
    if (objLabel == NULL) {
        return GUI_RET_FAILURE_NOMEM;
    }

    gui_label_t* elemLabel = calloc(1, sizeof(gui_label_t));
    if (elemLabel == NULL) {
        free(objLabel);
        return GUI_RET_FAILURE_NOMEM;
    }

    //initialize the object container
    objLabel->_elem = elemLabel;
    objLabel->_type = GUI_ELEMENT_LABEL;
    objLabel->_destructor = &Destructor;

    //initialize the element
    elemLabel->_pos.x = posx;
    elemLabel->_pos.y = posy;

    if (text != NULL) {
        char* strLabelText = malloc(strlen(text) + 1);
        if (strLabelText == NULL) {
            //malloc for the text string failed, button cannot be created as requested
            GUI_DestroyObject(objLabel);
            return NULL;
        }

        strcpy(strLabelText, text);
        elemLabel->_text = strLabelText;
    } else {
        elemLabel->_text = NULL;
    }

    return objLabel;
}

/* --- Private Functions --- */
static void Destructor(void* element) {
    gui_label_t* label = (gui_label_t*)element;
    free(label->_text);
    free(label);
    return;
}

static gui_ret_t SetPosition(gui_label_t* label, const int32_t posx, const int32_t posy) {
    if (label == NULL) {
        return GUI_RET_FAILURE_NULLPTR;
    }

    label->_pos.x = posx;
    label->_pos.y = posy;

    return GUI_RET_SUCCESS;
}

static gui_ret_t SetText(gui_label_t* label, const char* text) {
    if (label == NULL) {
        return GUI_RET_FAILURE_NULLPTR;
    }

    if (text == NULL) {
        free(label->_text);
        label->_text = NULL;
    } else {
        char* strNewText = malloc(strlen(text));
        if (strNewText == NULL) {
            return GUI_RET_FAILURE_NOMEM;
        }

        strcpy(strNewText, text);
        free(label->_text);
        label->_text = strNewText;
    }

    return GUI_RET_SUCCESS;
}
