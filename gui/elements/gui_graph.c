#include "gui/elements/gui_graph.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"
#include "utils/scope.h"

static void Destructor(void* this) {}

static void Render(void* this, gui_theme_t* theme, point_t origin) {
    gui_graph_t* thisGraph = (gui_graph_t*)this;
    point_t      posGraph  = GUI_Graph_GetPosition(thisGraph);
    rect_t       dimGraph  = GUI_Graph_GetDimensions(thisGraph);

    int32_t incXDivSpacing = dimGraph.w / GUI_GRAPH_DIVISIONS_X;
    int32_t incYDivSpacing = dimGraph.h / GUI_GRAPH_DIVISIONS_Y;

    const int32_t spaceText = 1;
    const int32_t scaleText = 1;
    char          bufTemp[8];

    // draw border
    SCR_DrawRectangle(
        origin.x + posGraph.x,
        origin.y + posGraph.y,
        origin.x + posGraph.x + dimGraph.w,
        origin.y + posGraph.y + dimGraph.h,
        false,
        theme->border);

    // draw x-axis division lines
    for (int32_t ii = 1; ii < GUI_GRAPH_DIVISIONS_X; ii++) {
        SCR_DrawLine(
            origin.x + posGraph.x + ii * incXDivSpacing,
            origin.y + posGraph.y + 1,
            origin.x + posGraph.x + ii * incXDivSpacing,
            origin.y + posGraph.y + dimGraph.h - 1,
            theme->subtle);
    }

    // draw y-axis division lines
    for (int32_t ii = 1; ii < GUI_GRAPH_DIVISIONS_Y; ii++) {
        SCR_DrawLine(
            origin.x + posGraph.x + 1,
            origin.y + posGraph.y + ii * incYDivSpacing,
            origin.x + posGraph.x + dimGraph.w - 1,
            origin.y + posGraph.y + ii * incYDivSpacing,
            theme->subtle);
    }

    // draw division x-labels for time
    float time        = thisGraph->_waveforms[0].x.lower;
    float incXDivTime = (thisGraph->_waveforms[0].x.upper - thisGraph->_waveforms[0].x.lower) / GUI_GRAPH_DIVISIONS_X;

    for (int32_t ii = 0; ii <= GUI_GRAPH_DIVISIONS_X; ii++) {
        snprintf(bufTemp, sizeof(bufTemp), "%4.0f", time);
        time += incXDivTime;

        SCR_DrawString(
            origin.x + posGraph.x + ii * incXDivSpacing - 2 * FONT_WIDTH,
            origin.y + posGraph.y + dimGraph.h + 5 * spaceText,
            bufTemp,
            scaleText,
            theme->text);
    }

    // draw division y-labels for CH0
    float volt = thisGraph->_waveforms[0].y.upper;
    float incYDivVoltsCH0 =
        (thisGraph->_waveforms[0].y.upper - thisGraph->_waveforms[0].y.lower) / GUI_GRAPH_DIVISIONS_Y;

    for (int32_t ii = 0; ii <= GUI_GRAPH_DIVISIONS_Y; ii++) {
        snprintf(bufTemp, sizeof(bufTemp), "%.1f", volt);
        volt -= incYDivVoltsCH0;

        SCR_DrawString(
            origin.x + posGraph.x - spaceText - 3 * FONT_WIDTH,
            origin.y + posGraph.y + ii * incYDivSpacing - 4,
            bufTemp,
            scaleText,
            theme->accents[0]);
    }

    // draw division y-labels for CH1
    volt = thisGraph->_waveforms[1].y.upper;
    float incYDivVoltsCH1 =
        (thisGraph->_waveforms[1].y.upper - thisGraph->_waveforms[1].y.lower) / GUI_GRAPH_DIVISIONS_Y;

    for (int32_t ii = 0; ii <= GUI_GRAPH_DIVISIONS_Y; ii++) {
        snprintf(bufTemp, sizeof(bufTemp), "%.1f", volt);
        volt -= incYDivVoltsCH1;

        SCR_DrawString(
            origin.x + posGraph.x + dimGraph.w + 9 * spaceText,
            origin.y + posGraph.y + ii * incYDivSpacing - 4,
            bufTemp,
            scaleText,
            theme->accents[1]);
    }

    // draw cursors
    if (thisGraph->_cursors != NULL) {
        linkedlist_t* cursorSel = LinkedList_HeadOf(thisGraph->_cursors);
        while (cursorSel != NULL) {
            float adjCursor = ((gui_cursor_t*)(cursorSel->val))->pos;
            if (adjCursor > thisGraph->_waveforms[0].x.upper) {
                adjCursor = thisGraph->_waveforms[0].x.upper;
            } else if (adjCursor < thisGraph->_waveforms[0].x.lower) {
                adjCursor = thisGraph->_waveforms[0].x.lower;
            }

            float posXNorm = (adjCursor - thisGraph->_waveforms[0].x.lower) /
                             (thisGraph->_waveforms[0].x.upper - thisGraph->_waveforms[0].x.lower);
            int32_t posXCursor = (int32_t)(dimGraph.w * posXNorm);
            SCR_DrawLine(
                origin.x + posGraph.x + posXCursor,
                origin.y + posGraph.y + 1,
                origin.x + posGraph.x + posXCursor,
                origin.y + posGraph.y + dimGraph.h - 1,
                theme->accents[2]);

            cursorSel = cursorSel->next;
        }
    }

    // draw waveform for CH1
    float spanVerticalCH1 = thisGraph->_waveforms[1].y.upper - thisGraph->_waveforms[1].y.lower;
    float incXPos         = (float)dimGraph.w / (float)(GUI_WAVEFORM_NUM_POINTS - 1);

    for (int32_t ii = 0; ii < GUI_WAVEFORM_NUM_POINTS - 1; ii++) {
        float adjY0 = thisGraph->_waveforms[1].samples[ii];
        float adjY1 = thisGraph->_waveforms[1].samples[ii + 1];

        if (adjY0 > thisGraph->_waveforms[1].y.upper) {
            adjY0 = thisGraph->_waveforms[1].y.upper;
        } else if (adjY0 < thisGraph->_waveforms[0].x.lower) {
            adjY0 = thisGraph->_waveforms[1].x.lower;
        }

        if (adjY1 > thisGraph->_waveforms[1].y.upper) {
            adjY1 = thisGraph->_waveforms[1].y.upper;
        } else if (adjY1 < thisGraph->_waveforms[0].x.lower) {
            adjY1 = thisGraph->_waveforms[1].x.lower;
        }

        float offsetY0, offsetY1;
        offsetY0 = thisGraph->_waveforms[1].samples[ii] - thisGraph->_waveforms[1].y.lower;
        offsetY1 = thisGraph->_waveforms[1].samples[ii + 1] - thisGraph->_waveforms[1].y.lower;

        float posNormY0 = offsetY0 / spanVerticalCH1;
        float posNormY1 = offsetY1 / spanVerticalCH1;

        int32_t posY0 = dimGraph.h * (1.0f - posNormY0);
        int32_t posY1 = dimGraph.h * (1.0f - posNormY1);

        int32_t posX0 = (int32_t)(incXPos * ii);
        int32_t posX1 = (int32_t)(incXPos * (ii + 1));

        SCR_DrawLine(
            origin.x + posGraph.x + posX0,
            origin.y + posGraph.y + posY0,
            origin.x + posGraph.x + posX1,
            origin.y + posGraph.y + posY1,
            theme->accents[1]);
    }

    // draw waveform for CH0
    float spanVerticalCH0 = thisGraph->_waveforms[0].y.upper - thisGraph->_waveforms[0].y.lower;
    incXPos               = (float)dimGraph.w / (float)(GUI_WAVEFORM_NUM_POINTS - 1);

    for (int32_t ii = 0; ii < GUI_WAVEFORM_NUM_POINTS - 1; ii++) {
        float adjY0 = thisGraph->_waveforms[0].samples[ii];
        float adjY1 = thisGraph->_waveforms[0].samples[ii + 1];

        if (adjY0 > thisGraph->_waveforms[0].y.upper) {
            adjY0 = thisGraph->_waveforms[0].y.upper;
        } else if (adjY0 < thisGraph->_waveforms[0].x.lower) {
            adjY0 = thisGraph->_waveforms[0].x.lower;
        }

        if (adjY1 > thisGraph->_waveforms[0].y.upper) {
            adjY1 = thisGraph->_waveforms[0].y.upper;
        } else if (adjY1 < thisGraph->_waveforms[0].x.lower) {
            adjY1 = thisGraph->_waveforms[0].x.lower;
        }

        float offsetY0, offsetY1;
        offsetY0 = thisGraph->_waveforms[0].samples[ii] - thisGraph->_waveforms[0].y.lower;
        offsetY1 = thisGraph->_waveforms[0].samples[ii + 1] - thisGraph->_waveforms[0].y.lower;

        float posNormY0 = offsetY0 / spanVerticalCH1;
        float posNormY1 = offsetY1 / spanVerticalCH1;

        int32_t posY0 = dimGraph.h * (1.0f - posNormY0);
        int32_t posY1 = dimGraph.h * (1.0f - posNormY1);

        int32_t posX0 = (int32_t)(incXPos * ii);
        int32_t posX1 = (int32_t)(incXPos * (ii + 1));

        SCR_DrawLine(
            origin.x + posGraph.x + posX0,
            origin.y + posGraph.y + posY0,
            origin.x + posGraph.x + posX1,
            origin.y + posGraph.y + posY1,
            theme->accents[0]);
    }

    return;
}

void (*GUI_Graph_GetDestructor(void))(void*) {
    return &Destructor;
}

gui_ret_t GUI_Graph_New(gui_graph_t* this, int32_t posx, int32_t posy, bool visible, int32_t width, int32_t height) {
    gui_ret_t ret;

    // initialize the base class
    ret = GUI_Object_New((gui_object_t*)this, posx, posy, visible);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // initialize the graph class
    ret = GUI_Graph_SetDimensions(this, width, height);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // replace override functions
    ((gui_object_t*)this)->_Render = &Render;

    // replace the destructor
    ((class_t*)this)->_Destructor = &Destructor;

    return ret;
}

gui_ret_t GUI_Graph_SetPosition(gui_graph_t* this, int32_t posx, int32_t posy) {
    return GUI_Object_SetPosition((gui_object_t*)this, posx, posy);
}

point_t GUI_Graph_GetPosition(gui_graph_t* this) {
    return GUI_Object_GetPosition((gui_object_t*)this);
}

gui_ret_t GUI_Graph_SetDimensions(gui_graph_t* this, int32_t width, int32_t height) {
    if (width < GUI_GRAPH_WIDTH_MIN || width > GUI_GRAPH_WIDTH_MAX || height < GUI_GRAPH_HEIGHT_MIN ||
        height > GUI_GRAPH_HEIGHT_MAX) {
        return GUI_RET_FAILURE_INVALID;
    }

    this->_dim.w = width;
    this->_dim.h = height;

    return GUI_RET_SUCCESS;
}

rect_t GUI_Graph_GetDimensions(gui_graph_t* this) {
    return this->_dim;
}

gui_waveform_t* GUI_Graph_GetWaveform(gui_graph_t* this, scope_channel_t channel) {
    return &this->_waveforms[channel];
}

linkedlist_t* GUI_Graph_AddCursor(gui_graph_t* this, float time) {
    linkedlist_t* listCursor = LinkedList_New();
    if (listCursor == NULL) {
        return NULL;
    }

    gui_cursor_t* guiCursor = calloc(1, sizeof(gui_cursor_t));
    if (guiCursor == NULL) {
        return NULL;
    }

    guiCursor->pos  = time;
    listCursor->val = guiCursor;

    if (this->_cursors == NULL) {
        this->_cursors = listCursor;
    } else {
        LinkedList_AppendNode(this->_cursors, listCursor);
    }

    return listCursor;
}

gui_ret_t GUI_Graph_UpdateCursor(gui_graph_t* this, linkedlist_t* cursor, float time) {
    if (this->_cursors == NULL) {
        return GUI_RET_FAILURE_MISSING;
    }

    linkedlist_t* listCursorSel = LinkedList_HeadOf(this->_cursors);
    while (listCursorSel != NULL) {
        if (listCursorSel == cursor) {
            ((gui_cursor_t*)(listCursorSel->val))->pos = time;
            return GUI_RET_SUCCESS;
        } else {
            listCursorSel = listCursorSel->next;
        }
    }

    return GUI_RET_FAILURE_MISSING;
}

gui_ret_t GUI_Graph_RemoveCursor(gui_graph_t* this, linkedlist_t* cursor) {
    free(cursor->val);
    this->_cursors = LinkedList_DeleteNode(cursor);

    return GUI_RET_SUCCESS;
}
