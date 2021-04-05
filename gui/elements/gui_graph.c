#include "gui/elements/gui_graph.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "gui/gui_base.h"
#include "utils/scope.h"
#include "utils/units.h"

static void Destructor(void* this) {}

static void Render(void* this, gui_theme_t* theme, point_t origin) {
    gui_graph_t* thisGraph = (gui_graph_t*)this;
    point_t      posGraph  = GUI_Graph_GetPosition(thisGraph);
    rect_t       dimGraph  = GUI_Graph_GetDimensions(thisGraph);

    int32_t incXDivSpacing = dimGraph.w / GUI_GRAPH_DIVISIONS_X;
    int32_t incYDivSpacing = dimGraph.h / GUI_GRAPH_DIVISIONS_Y;

    const int32_t spaceText = 1;
    const int32_t scaleText = 1;
    char          bufTemp[16];

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
        int32_t lineDashedLen = 1;
        int32_t lineGappedLen = 2;

        if (ii == GUI_GRAPH_DIVISIONS_X / 2) {
            lineDashedLen = 4;
        }

        SCR_DrawVerticalDashedLine(
            origin.x + posGraph.x + ii * incXDivSpacing,
            origin.y + posGraph.y + 1,
            origin.y + posGraph.y + dimGraph.h - 1,
            lineDashedLen,
            lineGappedLen,
            theme->subtle);
    }

    // draw y-axis division lines
    for (int32_t ii = 1; ii < GUI_GRAPH_DIVISIONS_Y; ii++) {
        int32_t lineDashedLen = 1;
        int32_t lineGappedLen = 2;

        if (ii == GUI_GRAPH_DIVISIONS_Y / 2) {
            lineDashedLen = 4;
        }

        SCR_DrawHorizontalDashedLine(
            origin.x + posGraph.x + 1,
            origin.x + posGraph.x + dimGraph.w - 1,
            origin.y + posGraph.y + ii * incYDivSpacing,
            lineDashedLen,
            lineGappedLen,
            theme->subtle);
    }

    // draw division x-labels for time
    float  incXDivTime = (thisGraph->_waveforms[0].x.upper - thisGraph->_waveforms[0].x.lower) / GUI_GRAPH_DIVISIONS_X;
    unit_t unitTime    = Units_CalcIdealUnits(incXDivTime);
    char*  prefixTime  = Units_GetUnitPrefix(unitTime);
    float  multTime    = Units_GetUnitMultiplier(unitTime);

    float time = multTime * thisGraph->_waveforms[0].x.lower;
    incXDivTime *= multTime;

    for (int32_t ii = 0; ii <= GUI_GRAPH_DIVISIONS_X; ii++) {
        if (fabs(time) < 0.09f) {
            time = 0.0f;
        }

        snprintf(bufTemp, sizeof(bufTemp), "%6.1f %ss", time, prefixTime);
        time += incXDivTime;

        int32_t heightLabel = origin.y + posGraph.y + dimGraph.h + 5 * spaceText;
        if (ii % 2 == 0) {
            heightLabel += FONT_HEIGHT + 2;
        }

        SCR_DrawString(
            origin.x + posGraph.x + ii * incXDivSpacing - 4 * FONT_WIDTH,
            heightLabel,
            bufTemp,
            scaleText,
            theme->text);
    }

    // draw division y-labels for CH0
    float volt = thisGraph->_waveforms[0].y.upper;
    float incYDivVoltsCH0 =
        (thisGraph->_waveforms[0].y.upper - thisGraph->_waveforms[0].y.lower) / GUI_GRAPH_DIVISIONS_Y;

    for (int32_t ii = 0; ii <= GUI_GRAPH_DIVISIONS_Y; ii++) {
        snprintf(bufTemp, sizeof(bufTemp), "%6.2f", volt);
        volt -= incYDivVoltsCH0;

        SCR_DrawString(
            origin.x + posGraph.x - spaceText - 6 * FONT_WIDTH,
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
        snprintf(bufTemp, sizeof(bufTemp), "%6.2f", volt);
        volt -= incYDivVoltsCH1;

        SCR_DrawString(
            origin.x + posGraph.x + dimGraph.w + 7 * spaceText,
            origin.y + posGraph.y + ii * incYDivSpacing - 4,
            bufTemp,
            scaleText,
            theme->accents[1]);
    }

    // draw waveform for CH1
    float spanVerticalCH1 = thisGraph->_waveforms[1].y.upper - thisGraph->_waveforms[1].y.lower;
    float incXPos         = (float)dimGraph.w / (float)(GUI_WAVEFORM_NUM_POINTS - 1);

    for (int32_t ii = 0; ii < GUI_WAVEFORM_NUM_POINTS - 1; ii++) {
        float adjY0 = thisGraph->_waveforms[1].samples[ii];
        float adjY1 = thisGraph->_waveforms[1].samples[ii + 1];

        if (adjY0 >= thisGraph->_waveforms[1].y.upper) {
            adjY0 = thisGraph->_waveforms[1].y.upper;
        } else if (adjY0 <= thisGraph->_waveforms[1].y.lower) {
            adjY0 = thisGraph->_waveforms[1].y.lower;
        }

        if (adjY1 >= thisGraph->_waveforms[1].y.upper) {
            adjY1 = thisGraph->_waveforms[1].y.upper;
        } else if (adjY1 <= thisGraph->_waveforms[1].y.lower) {
            adjY1 = thisGraph->_waveforms[1].y.lower;
        }

        float offsetY0, offsetY1;
        offsetY0 = adjY0 - thisGraph->_waveforms[1].y.lower;
        offsetY1 = adjY1 - thisGraph->_waveforms[1].y.lower;

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

        if (adjY0 >= thisGraph->_waveforms[0].y.upper) {
            adjY0 = thisGraph->_waveforms[0].y.upper;
        } else if (adjY0 <= thisGraph->_waveforms[0].y.lower) {
            adjY0 = thisGraph->_waveforms[0].y.lower;
        }

        if (adjY1 >= thisGraph->_waveforms[0].y.upper) {
            adjY1 = thisGraph->_waveforms[0].y.upper;
        } else if (adjY1 <= thisGraph->_waveforms[0].y.lower) {
            adjY1 = thisGraph->_waveforms[0].y.lower;
        }

        float offsetY0, offsetY1;
        offsetY0 = adjY0 - thisGraph->_waveforms[0].y.lower;
        offsetY1 = adjY1 - thisGraph->_waveforms[0].y.lower;

        float posNormY0 = offsetY0 / spanVerticalCH0;
        float posNormY1 = offsetY1 / spanVerticalCH0;

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

    // draw cursors
    if (thisGraph->_cursors != NULL) {
        linkedlist_t* nodeCursorSel = LinkedList_HeadOf(thisGraph->_cursors);

        float spanTime = thisGraph->_waveforms[0].x.upper - thisGraph->_waveforms[0].x.lower;

        while (nodeCursorSel != NULL) {
            float normPosCursor =
                (((gui_cursor_t*)(nodeCursorSel->val))->pos - thisGraph->_waveforms[0].x.lower) / spanTime;
            int32_t posCursor = normPosCursor * dimGraph.w;

            if (posCursor < 1) {
                posCursor = 1;
            } else if (posCursor >= dimGraph.w) {
                posCursor = dimGraph.w - 1;
            }

            SCR_DrawLine(
                origin.x + posGraph.x + posCursor,
                origin.y + posGraph.y + 1,
                origin.x + posGraph.x + posCursor,
                origin.y + posGraph.y + dimGraph.h - 1,
                theme->accents[2]);

            nodeCursorSel = nodeCursorSel->next;
        }
    }

    // draw triggers
    for (int32_t ii = 0; ii < GUI_GRAPH_NUM_WAVEFORMS; ii++) {
        float spanVertical = 0;
        if (ii == 0) {
            spanVertical = spanVerticalCH0;
        } else if (ii == 1) {
            spanVertical = spanVerticalCH1;
        }

        float normTrigCenterY =
            1 - ((thisGraph->_waveforms[ii].trigger - thisGraph->_waveforms[ii].y.lower) / spanVertical);
        int32_t posTrigCenterY = normTrigCenterY * dimGraph.h;
        if (posTrigCenterY < 1) {
            posTrigCenterY = 1;
        } else if (posTrigCenterY > dimGraph.h - 1) {
            posTrigCenterY = dimGraph.h - 1;
        }

        int32_t posTrigUpperY = posTrigCenterY - 4;
        if (posTrigUpperY < 1) {
            posTrigUpperY = 1;
        }

        int32_t posTrigLowerY = posTrigCenterY + 4;
        if (posTrigLowerY > dimGraph.h - 1) {
            posTrigLowerY = dimGraph.h - 1;
        }

        SCR_DrawTriangle(
            origin.x + posGraph.x + dimGraph.w - 8,
            origin.y + posGraph.y + posTrigCenterY,
            origin.x + posGraph.x + dimGraph.w,
            origin.y + posGraph.y + posTrigLowerY,
            origin.x + posGraph.x + dimGraph.w,
            origin.y + posGraph.y + posTrigUpperY,
            true,
            theme->accents[ii]);
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
