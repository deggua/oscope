#include "core/sys.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display/display.h"
#include "drivers/adc.h"
#include "drivers/dac.h"
#include "drivers/spi.h"
#include "drivers/user_controls.h"
#include "gui/elements/gui_button.h"
#include "gui/elements/gui_graph.h"
#include "gui/elements/gui_label.h"
#include "gui/elements/gui_window.h"
#include "gui/gui.h"
#include "gui/gui_base.h"
#include "gui/gui_interactor.h"
#include "processing/control.h"

#define ADC_FREQ (275.0e6 / 11.0 / (49.0 + 1.0))

#define HORIZONTAL_SCALE_STEP_COARSE 10
#define HORIZONTAL_SCALE_STEP_FINE   1
#define VERTICAL_SCALE_STEP_COARSE   1.0f
#define VERTICAL_SCALE_STEP_FINE     0.1f
#define VERTICAL_SPAN_MIN            0.1f
#define VERTICAL_SPAN_MAX            33.0f
#define OFFSET_STEP_FINE             0.1f
#define OFFSET_STEP_COARSE           0.5f
#define OFFSET_MIN                   -17.0f
#define OFFSET_MAX                   17.0f
#define TRIGGER_STEP_FINE            0.1f
#define TRIGGER_STEP_COARSE          0.5f
#define TRIGGER_MAX                  (33.0f / 2.0f)
#define TRIGGER_MIN                  -TRIGGER_MAX
#define CURSOR_POSITIONS             80

// ticker reference
TIM_HandleTypeDef* g_tick;

// GUI references
gui_waveform_t *  g_waveCH0, *g_waveCH1;
gui_graph_t*      g_guiGraph;
gui_window_t*     g_guiWindSettings;
gui_window_t*     g_guiWindRoot;
gui_theme_t*      g_theme;
gui_interactor_t* g_interSel;
gui_label_t *     g_guiLabelCH0, *g_guiLabelCH1;
gui_cursor_t*     g_guiCursorSel;
gui_button_t *    g_guiButtonTriggerCH0, *g_guiButtonTriggerCH1;

// ticker counters
volatile uint32_t g_tickRender          = 0;
volatile uint32_t g_tickSampleControls  = 0;
volatile uint32_t g_tickProcessControls = 0;
volatile uint32_t g_tickSampleADC       = 0;

// adc ticker prescaler
volatile uint32_t g_tickSampleADCPrescaler = 0;

// CH0 globals
volatile float       g_voltADCSamplesCH0[ADC_SAMPLES_PER_WAVEFORM];
volatile size_t      g_indexADCSamplesCH0     = 0;
volatile size_t      g_lenADCAfterTriggerCH0  = 0;
volatile size_t      g_lenADCBeforeTriggerCH0 = 0;
volatile adc_state_t g_stateCH0               = ADC_RUNNING;
volatile trigger_t   g_triggerCH0             = {.edge = EDGE_RISING, .threshold = 0.0f};
volatile scope_rot_t g_activityRotCH0         = ROT_HORIZONTAL_SCALE;

// CH1 globals
volatile float       g_voltADCSamplesCH1[ADC_SAMPLES_PER_WAVEFORM];
volatile size_t      g_indexADCSamplesCH1     = 0;
volatile size_t      g_lenADCBeforeTriggerCH1 = 0;
volatile size_t      g_lenADCAfterTriggerCH1  = 0;
volatile adc_state_t g_stateCH1               = ADC_RUNNING;
volatile trigger_t   g_triggerCH1             = {.edge = EDGE_RISING, .threshold = 0.0f};
volatile scope_rot_t g_activityRotCH1         = ROT_HORIZONTAL_SCALE;

static void      CORE_FrontEnd_Init(void);
static gui_ret_t CORE_GUI_Init(void);

static inline void Callback_Conf(void);
static inline void Callback_Cursor(void);
static inline void Callback_TriggerCH0(void);
static inline void Callback_TriggerCH1(void);

static inline range_t TimeRangeFromFreq(float freq);
static inline void    UpdateTriggerText(gui_button_t* button, trigger_t trig);

static inline bool isTriggerTripped(float prevSample, float curSample, trigger_t* trig) {
    switch (trig->edge) {
        case EDGE_RISING: {
            if (curSample > trig->threshold && prevSample < trig->threshold) {
                return true;
            }
        } break;

        case EDGE_FALLING: {
            if (curSample < trig->threshold && prevSample > trig->threshold) {
                return true;
            }
        } break;

        case EDGE_BOTH: {
            if (curSample > trig->threshold && prevSample < trig->threshold ||
                curSample < trig->threshold && prevSample > trig->threshold) {
                return true;
            }
        } break;

        default: break;
    }

    return false;
}

static inline void CopyCircularWaveform(float* dst, float* src, size_t idx_first, size_t len) {
    size_t indexCur = idx_first;
    for (int32_t ii = 0; ii < len; ii++) {
        dst[ii]  = src[indexCur];
        indexCur = (indexCur + 1) % len;
    }

    return;
}

static inline void StoreADCSample(scope_channel_t channel) {
    if (channel == SCOPE_CH0) {
        g_voltADCSamplesCH0[g_indexADCSamplesCH0] = PROC_SampleVoltageCalibrated(SCOPE_CH0);
    } else if (channel == SCOPE_CH1) {
        g_voltADCSamplesCH1[g_indexADCSamplesCH1] = PROC_SampleVoltageCalibrated(SCOPE_CH1);
    }
    return;
}

static inline void OperationUpdateGUI(void) {
    // update waveforms
    if (g_stateCH0 == ADC_AVAILABLE) {
        CopyCircularWaveform(g_waveCH0->samples, g_voltADCSamplesCH0, g_indexADCSamplesCH0, ADC_SAMPLES_PER_WAVEFORM);
        g_stateCH0 = ADC_RUNNING;
    }

    if (g_stateCH1 == ADC_AVAILABLE) {
        CopyCircularWaveform(g_waveCH1->samples, g_voltADCSamplesCH1, g_indexADCSamplesCH1, ADC_SAMPLES_PER_WAVEFORM);
        g_stateCH1 = ADC_RUNNING;
    }

    // Redraw the GUI
    GUI_Object_Render((gui_object_t*)g_guiWindRoot, g_theme, (point_t){.x = 0, .y = 0});
}

static inline float AddWithinMaximum_float(float value, float increment, float maximum) {
    if (value < maximum - increment) {
        value += increment;
    } else {
        value = maximum;
    }

    return value;
}

static inline float SubWithinMinimum_float(float value, float decrement, float minimum) {
    if (value > minimum + decrement) {
        value -= decrement;
    } else {
        value = minimum;
    }

    return value;
}

static inline uint32_t AddWithinMaximum_u32(uint32_t value, uint32_t increment, uint32_t maximum) {
    if (value < maximum - increment) {
        value += increment;
    } else {
        value = maximum;
    }

    return value;
}

static inline uint32_t SubWithinMinimum_u32(uint32_t value, uint32_t decrement, uint32_t minimum) {
    if (value > minimum + decrement) {
        value -= decrement;
    } else {
        value = minimum;
    }

    return value;
}

static inline void UpdateChannelLabel(gui_label_t* label, scope_channel_t channel, scope_rot_t state) {
    char bufTemp[32] = {0};

    if (channel == SCOPE_CH0) {
        strcpy(bufTemp, "CH0: ");
    } else if (channel == SCOPE_CH1) {
        strcpy(bufTemp, "CH1: ");
    }

    switch (state) {
        case ROT_HORIZONTAL_SCALE: {
            strcat(bufTemp, "HORZ COARSE");
        } break;

        case ROT_HORIZONTAL_SCALE_FINE: {
            strcat(bufTemp, "HORZ FINE");
        } break;

        case ROT_VERTICAL_SCALE: {
            strcat(bufTemp, "VERT COARSE");
        } break;

        case ROT_VERTICAL_SCALE_FINE: {
            strcat(bufTemp, "VERT FINE");
        } break;

        case ROT_OFFSET_VOLTAGE: {
            strcat(bufTemp, "OFST COARSE");
        } break;

        case ROT_OFFSET_VOLTAGE_FINE: {
            strcat(bufTemp, "OFST FINE");
        } break;

        case ROT_TRIGGER_VOLTAGE: {
            strcat(bufTemp, "TRIG COARSE");
        } break;

        case ROT_TRIGGER_VOLTAGE_FINE: {
            strcat(bufTemp, "TRIG FINE");
        } break;

        case ROT_CURSOR: {
            strcat(bufTemp, "CURSOR");
        } break;

        default: break;
    }

    GUI_Label_SetText(label, bufTemp);
    return;
}

static inline void UpdateTriggerText(gui_button_t* button, trigger_t trig) {
    if (trig.edge == EDGE_RISING) {
        GUI_Button_SetText(button, "RISE");
    } else if (trig.edge == EDGE_FALLING) {
        GUI_Button_SetText(button, "FALL");
    } else if (trig.edge == EDGE_BOTH) {
        GUI_Button_SetText(button, "BOTH");
    }
    return;
}

static inline void Callback_TriggerCH0(void) {
    g_triggerCH0.edge = (g_triggerCH0.edge + 1) % EDGE_LAST;
    UpdateTriggerText(g_guiButtonTriggerCH0, g_triggerCH0);
    return;
}

static inline void Callback_TriggerCH1(void) {
    g_triggerCH1.edge = (g_triggerCH1.edge + 1) % EDGE_LAST;
    UpdateTriggerText(g_guiButtonTriggerCH1, g_triggerCH1);
    return;
}

static inline void Callback_Conf(void) {}

static inline void Callback_Cursor(void) {
    static linkedlist_t* cursors[2] = {0};
    static size_t        idxCursor  = 0;

    if (idxCursor < lengthof(cursors)) {
        linkedlist_t* cursor = GUI_Graph_AddCursor(g_guiGraph, 0.0f);
        cursors[idxCursor++] = cursor;
        g_guiCursorSel       = (gui_cursor_t*)(cursor->val);
        g_activityRotCH0     = ROT_CURSOR;
        UpdateChannelLabel(g_guiLabelCH0, SCOPE_CH0, g_activityRotCH0);
    } else {
        idxCursor = 0;
        for (int32_t ii = 0; ii < lengthof(cursors); ii++) {
            GUI_Graph_RemoveCursor(g_guiGraph, cursors[ii]);
        }
    }

    return;
}

// Callback: timer has rolled over
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    if (htim == g_tick) {
        g_tickRender++;
        g_tickSampleControls++;
        g_tickProcessControls++;
        g_tickSampleADC++;

        // check if we've reached the prescaler value of ticks
        if (g_tickSampleADC > g_tickSampleADCPrescaler) {
            // reset adc ticker
            g_tickSampleADC = 0;

            // process adc state for CH0
            if (g_stateCH0 == ADC_RUNNING) {
                // store ADC sample
                StoreADCSample(SCOPE_CH0);
                g_lenADCBeforeTriggerCH0++;

                if (g_lenADCBeforeTriggerCH0 > ADC_SAMPLES_PER_WAVEFORM / 2) {
                    float  curSampleCH0 = g_voltADCSamplesCH0[g_indexADCSamplesCH0];
                    size_t prevIndexCH0 =
                        (g_indexADCSamplesCH0 + lengthof(g_voltADCSamplesCH0) - 1) % lengthof(g_voltADCSamplesCH0);
                    float prevSampleCH0 = g_voltADCSamplesCH0[prevIndexCH0];

                    // process trigger for CH0
                    if (isTriggerTripped(prevSampleCH0, curSampleCH0, &g_triggerCH0)) {
                        g_lenADCAfterTriggerCH0  = 0;
                        g_lenADCBeforeTriggerCH0 = 0;
                        g_stateCH0               = ADC_TRIGGERED;
                    }
                }

                if (g_lenADCBeforeTriggerCH0 >= 8 * ADC_SAMPLES_PER_WAVEFORM) {
                    g_lenADCAfterTriggerCH0  = 0;
                    g_lenADCBeforeTriggerCH0 = 0;
                    g_stateCH0               = ADC_TRIGGERED;
                }

                g_indexADCSamplesCH0 = (g_indexADCSamplesCH0 + 1) % lengthof(g_voltADCSamplesCH0);
            } else if (g_stateCH0 == ADC_TRIGGERED && g_lenADCAfterTriggerCH0 < ADC_SAMPLES_PER_WAVEFORM / 2 - 1) {
                // store ADC sample
                g_voltADCSamplesCH0[g_indexADCSamplesCH0] = PROC_SampleVoltageCalibrated(SCOPE_CH0);
                g_lenADCAfterTriggerCH0++;
                g_indexADCSamplesCH0 = (g_indexADCSamplesCH0 + 1) % lengthof(g_voltADCSamplesCH0);
            } else {
                // mark that a full waveform is available
                g_stateCH0 = ADC_AVAILABLE;
            }

            // process adc state for CH0
            if (g_stateCH1 == ADC_RUNNING) {
                // store ADC sample
                StoreADCSample(SCOPE_CH1);
                g_lenADCBeforeTriggerCH1++;

                if (g_lenADCBeforeTriggerCH1 > ADC_SAMPLES_PER_WAVEFORM / 2) {
                    float  curSampleCH1 = g_voltADCSamplesCH1[g_indexADCSamplesCH1];
                    size_t prevIndexCH1 =
                        (g_indexADCSamplesCH1 + lengthof(g_voltADCSamplesCH1) - 1) % lengthof(g_voltADCSamplesCH1);
                    float prevSampleCH1 = g_voltADCSamplesCH1[prevIndexCH1];

                    // process trigger for CH0
                    if (isTriggerTripped(prevSampleCH1, curSampleCH1, &g_triggerCH1)) {
                        g_lenADCAfterTriggerCH1  = 0;
                        g_lenADCBeforeTriggerCH1 = 0;
                        g_stateCH1               = ADC_TRIGGERED;
                    }
                }

                if (g_lenADCBeforeTriggerCH1 >= 8 * ADC_SAMPLES_PER_WAVEFORM) {
                    g_lenADCAfterTriggerCH1  = 0;
                    g_lenADCBeforeTriggerCH1 = 0;
                    g_stateCH1               = ADC_TRIGGERED;
                }

                g_indexADCSamplesCH1 = (g_indexADCSamplesCH1 + 1) % lengthof(g_voltADCSamplesCH1);
            } else if (g_stateCH1 == ADC_TRIGGERED && g_lenADCAfterTriggerCH1 < ADC_SAMPLES_PER_WAVEFORM / 2 - 1) {
                // store ADC sample
                g_voltADCSamplesCH1[g_indexADCSamplesCH1] = PROC_SampleVoltageCalibrated(SCOPE_CH1);
                g_lenADCAfterTriggerCH1++;
                g_indexADCSamplesCH1 = (g_indexADCSamplesCH1 + 1) % lengthof(g_voltADCSamplesCH1);
            } else {
                // mark that a full waveform is available
                g_stateCH1 = ADC_AVAILABLE;
            }
        }
    }
}

void CORE_Sys_Init(TIM_HandleTypeDef* tick, IRQn_Type irq) {
    // save pointers
    g_tick = tick;

    // initialize the analog front end
    CORE_FrontEnd_Init();

    // initialize the GUI
    CORE_GUI_Init();

    // initialize ticker and interrupt
    HAL_TIM_Base_Start_IT(g_tick);
    HAL_NVIC_SetPriority(irq, 0, 0);
    HAL_NVIC_EnableIRQ(irq);

    return;
}

void CORE_Sys_Run(void) {
    const uint32_t tickRenderPeriod          = ADC_FREQ / 30;   // 30 Hz redraw
    const uint32_t tickSampleControlsPeriod  = ADC_FREQ / 1000; // 1000 Hz sampling
    const uint32_t tickProcessControlsPeriod = ADC_FREQ / 10;   // 10 Hz execution of controls

    joy_state_t curJoyState    = {0};
    joy_state_t prevJoyState   = {0};
    bool        isJoyProcessed = true;

    rot_state_t curRotStateCH0    = {0};
    rot_state_t prevRotStateCH0   = {0};
    bool        isRotCH0Processed = true;

    rot_state_t curRotStateCH1    = {0};
    rot_state_t prevRotStateCH1   = {0};
    bool        isRotCH1Processed = true;

    float spanVertCH0   = 10.0f;
    float offsetVertCH0 = 0.0f;
    float spanVertCH1   = 10.0f;
    float offsetVertCH1 = 0.0f;

    range_t newRange   = TimeRangeFromFreq(ADC_FREQ / (g_tickSampleADCPrescaler + 2));
    g_waveCH0->x.lower = newRange.lower;
    g_waveCH0->x.upper = newRange.upper;

    g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f + offsetVertCH0;
    g_waveCH0->y.upper = spanVertCH0 / 2.0f + offsetVertCH0;

    PROC_SetSpan(SCOPE_CH0, spanVertCH0);
    PROC_SetOffset(SCOPE_CH0, offsetVertCH0);

    g_waveCH1->y.lower = -1.0f * spanVertCH1 / 2.0f + offsetVertCH1;
    g_waveCH1->y.upper = spanVertCH1 / 2.0f + offsetVertCH1;

    PROC_SetSpan(SCOPE_CH1, spanVertCH1);
    PROC_SetOffset(SCOPE_CH1, offsetVertCH1);

    while (1) {
        if (g_tickRender > tickRenderPeriod) {
            OperationUpdateGUI();
            g_tickRender = 0;
        }

        if (g_tickSampleControls > tickSampleControlsPeriod) {
            // sample user controls
            if (isJoyProcessed) {
                prevJoyState = curJoyState;
                curJoyState  = DRV_Joy_GetState();
                if (curJoyState.dir != JOY_DIR_NONE || curJoyState.pressed) {
                    isJoyProcessed = false;
                }
            }

            if (isRotCH0Processed) {
                prevRotStateCH0 = curRotStateCH0;
                curRotStateCH0  = DRV_Rot_GetState(SCOPE_CH0);
                if (curRotStateCH0.dir != ROT_DIR_NONE || curRotStateCH0.pressed) {
                    isRotCH0Processed = false;
                }
            }

            if (isRotCH1Processed) {
                prevRotStateCH1 = curRotStateCH1;
                curRotStateCH1  = DRV_Rot_GetState(SCOPE_CH1);
                if (curRotStateCH1.dir != ROT_DIR_NONE || curRotStateCH1.pressed) {
                    isRotCH1Processed = false;
                }
            }

            g_tickSampleControls = 0;
        }

        if (g_tickProcessControls > tickProcessControlsPeriod) {
            // execute based on user controls values
            // joy stick actions
            if (!isJoyProcessed) {
                if (curJoyState.pressed && !prevJoyState.pressed) {
                    GUI_Interactor_Execute(g_interSel);
                } else if (curJoyState.dir != JOY_DIR_NONE && prevJoyState.dir == JOY_DIR_NONE) {
                    g_interSel = GUI_Interactor_Select(g_interSel, curJoyState.dir);
                }
                isJoyProcessed = true;
            }

            // rot CH0 actions
            if (!isRotCH0Processed) {
                // process rotation actions
                if (g_activityRotCH0 == ROT_HORIZONTAL_SCALE || g_activityRotCH0 == ROT_HORIZONTAL_SCALE_FINE) {
                    // process horizontal (time) axis adjustments
                    uint32_t step;
                    if (g_activityRotCH0 == ROT_HORIZONTAL_SCALE) {
                        step = HORIZONTAL_SCALE_STEP_COARSE;
                    } else if (g_activityRotCH0 == ROT_HORIZONTAL_SCALE_FINE) {
                        step = HORIZONTAL_SCALE_STEP_FINE;
                    }

                    bool isTimeBaseChanged = false;
                    if (curRotStateCH0.dir == ROT_DIR_CW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_tickSampleADCPrescaler = AddWithinMaximum_u32(g_tickSampleADCPrescaler, step, UINT32_MAX);
                        isTimeBaseChanged        = true;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_tickSampleADCPrescaler = SubWithinMinimum_u32(g_tickSampleADCPrescaler, step, 0);
                        isTimeBaseChanged        = true;
                    }

                    if (isTimeBaseChanged) {
                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / (g_tickSampleADCPrescaler + 2));
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    }
                } else if (g_activityRotCH0 == ROT_VERTICAL_SCALE || g_activityRotCH0 == ROT_VERTICAL_SCALE_FINE) {
                    // process vertical (voltage) axis adjustments
                    float step;
                    if (g_activityRotCH0 == ROT_VERTICAL_SCALE) {
                        step = VERTICAL_SCALE_STEP_COARSE;
                    } else if (g_activityRotCH0 == ROT_VERTICAL_SCALE_FINE) {
                        step = VERTICAL_SCALE_STEP_FINE;
                    }

                    bool isVerticalScaleChanged = false;
                    if (curRotStateCH0.dir == ROT_DIR_CW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        spanVertCH0            = AddWithinMaximum_float(spanVertCH0, step, VERTICAL_SPAN_MAX);
                        isVerticalScaleChanged = true;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        spanVertCH0            = SubWithinMinimum_float(spanVertCH0, step, VERTICAL_SPAN_MIN);
                        isVerticalScaleChanged = true;
                    }

                    if (isVerticalScaleChanged) {
                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f + offsetVertCH0;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f + offsetVertCH0;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    }
                } else if (g_activityRotCH0 == ROT_OFFSET_VOLTAGE || g_activityRotCH0 == ROT_OFFSET_VOLTAGE_FINE) {
                    // process offset voltage adjustments
                    float step;
                    if (g_activityRotCH0 == ROT_OFFSET_VOLTAGE) {
                        step = OFFSET_STEP_COARSE;
                    } else if (g_activityRotCH0 == ROT_OFFSET_VOLTAGE_FINE) {
                        step = OFFSET_STEP_FINE;
                    }

                    bool isOffsetVoltageChanged = false;
                    if (curRotStateCH0.dir == ROT_DIR_CW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        offsetVertCH0          = AddWithinMaximum_float(offsetVertCH0, OFFSET_STEP_COARSE, OFFSET_MAX);
                        isOffsetVoltageChanged = true;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        offsetVertCH0          = SubWithinMinimum_float(offsetVertCH0, OFFSET_STEP_COARSE, OFFSET_MIN);
                        isOffsetVoltageChanged = true;
                    }

                    if (isOffsetVoltageChanged) {
                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f + offsetVertCH0;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f + offsetVertCH0;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    }
                } else if (g_activityRotCH0 == ROT_TRIGGER_VOLTAGE || g_activityRotCH0 == ROT_TRIGGER_VOLTAGE_FINE) {
                    // process trigger voltage adjustments
                    float step;
                    if (g_activityRotCH0 == ROT_TRIGGER_VOLTAGE) {
                        step = TRIGGER_STEP_COARSE;
                    } else if (g_activityRotCH0 == ROT_TRIGGER_VOLTAGE_FINE) {
                        step = TRIGGER_STEP_FINE;
                    }

                    bool isTriggerChanged = false;
                    if (curRotStateCH0.dir == ROT_DIR_CW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_triggerCH0.threshold = AddWithinMaximum_float(g_triggerCH0.threshold, step, TRIGGER_MAX);
                        isTriggerChanged = true;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_triggerCH0.threshold = SubWithinMinimum_float(g_triggerCH0.threshold, step, TRIGGER_MIN);
                        isTriggerChanged = true;
                    }

                    if (isTriggerChanged) {
                        g_waveCH0->trigger = g_triggerCH0.threshold;
                    }
                } else if (g_activityRotCH0 == ROT_CURSOR) {
                    // process adjustment of cursor position
                    float spanTime   = g_waveCH0->x.upper - g_waveCH0->x.lower;
                    float stepCursor = spanTime / CURSOR_POSITIONS;
                    float minCursor  = g_waveCH0->x.lower;
                    float maxCursor  = g_waveCH0->x.upper;

                    if (curRotStateCH0.dir == ROT_DIR_CW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_guiCursorSel->pos = AddWithinMaximum_float(g_guiCursorSel->pos, stepCursor, maxCursor);
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW && prevRotStateCH0.dir == ROT_DIR_NONE) {
                        g_guiCursorSel->pos = SubWithinMinimum_float(g_guiCursorSel->pos, stepCursor, minCursor);
                    }
                }

                // process push button action
                if (curRotStateCH0.pressed && !prevRotStateCH0.pressed) {
                    if (g_activityRotCH0 != ROT_CURSOR) {
                        // go to next rot state
                        g_activityRotCH0 = (g_activityRotCH0 + 1) % ROT_LAST_NORM;
                    } else if (g_activityRotCH0 == ROT_CURSOR) {
                        // exit place cursor state
                        g_activityRotCH0 = ROT_HORIZONTAL_SCALE;
                    }

                    UpdateChannelLabel(g_guiLabelCH0, SCOPE_CH0, g_activityRotCH0);
                }

                isRotCH0Processed = true;
            }

            // rot CH1 actions
            if (!isRotCH1Processed) {
                // process rotation actions
                if (g_activityRotCH1 == ROT_HORIZONTAL_SCALE || g_activityRotCH1 == ROT_HORIZONTAL_SCALE_FINE) {
                    // process horizontal (time) axis adjustments
                    uint32_t step;
                    if (g_activityRotCH1 == ROT_HORIZONTAL_SCALE) {
                        step = HORIZONTAL_SCALE_STEP_COARSE;
                    } else if (g_activityRotCH1 == ROT_HORIZONTAL_SCALE_FINE) {
                        step = HORIZONTAL_SCALE_STEP_FINE;
                    }

                    bool isTimeBaseChanged = false;
                    if (curRotStateCH1.dir == ROT_DIR_CW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        g_tickSampleADCPrescaler = AddWithinMaximum_u32(g_tickSampleADCPrescaler, step, UINT32_MAX);
                        isTimeBaseChanged        = true;
                    } else if (curRotStateCH1.dir == ROT_DIR_CCW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        g_tickSampleADCPrescaler = SubWithinMinimum_u32(g_tickSampleADCPrescaler, step, 0);
                        isTimeBaseChanged        = true;
                    }

                    if (isTimeBaseChanged) {
                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / (g_tickSampleADCPrescaler + 2));
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    }
                } else if (g_activityRotCH1 == ROT_VERTICAL_SCALE || g_activityRotCH1 == ROT_VERTICAL_SCALE_FINE) {
                    // process vertical (voltage) axis adjustments
                    float step;
                    if (g_activityRotCH1 == ROT_VERTICAL_SCALE) {
                        step = VERTICAL_SCALE_STEP_COARSE;
                    } else if (g_activityRotCH1 == ROT_VERTICAL_SCALE_FINE) {
                        step = VERTICAL_SCALE_STEP_FINE;
                    }

                    bool isVerticalScaleChanged = false;
                    if (curRotStateCH1.dir == ROT_DIR_CW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        spanVertCH1            = AddWithinMaximum_float(spanVertCH1, step, VERTICAL_SPAN_MAX);
                        isVerticalScaleChanged = true;
                    } else if (curRotStateCH1.dir == ROT_DIR_CCW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        spanVertCH1            = SubWithinMinimum_float(spanVertCH1, step, VERTICAL_SPAN_MIN);
                        isVerticalScaleChanged = true;
                    }

                    if (isVerticalScaleChanged) {
                        g_waveCH1->y.lower = -1.0f * spanVertCH1 / 2.0f + offsetVertCH1;
                        g_waveCH1->y.upper = spanVertCH1 / 2.0f + offsetVertCH1;

                        PROC_SetSpan(SCOPE_CH1, spanVertCH1);
                        PROC_SetOffset(SCOPE_CH1, offsetVertCH1);
                    }
                } else if (g_activityRotCH1 == ROT_OFFSET_VOLTAGE || g_activityRotCH1 == ROT_OFFSET_VOLTAGE_FINE) {
                    // process offset voltage adjustments
                    float step;
                    if (g_activityRotCH1 == ROT_OFFSET_VOLTAGE) {
                        step = OFFSET_STEP_COARSE;
                    } else if (g_activityRotCH1 == ROT_OFFSET_VOLTAGE_FINE) {
                        step = OFFSET_STEP_FINE;
                    }

                    bool isOffsetVoltageChanged = false;
                    if (curRotStateCH1.dir == ROT_DIR_CW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        offsetVertCH1          = AddWithinMaximum_float(offsetVertCH1, OFFSET_STEP_COARSE, OFFSET_MAX);
                        isOffsetVoltageChanged = true;
                    } else if (curRotStateCH1.dir == ROT_DIR_CCW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        offsetVertCH1          = SubWithinMinimum_float(offsetVertCH1, OFFSET_STEP_COARSE, OFFSET_MIN);
                        isOffsetVoltageChanged = true;
                    }

                    if (isOffsetVoltageChanged) {
                        g_waveCH1->y.lower = -1.0f * spanVertCH1 / 2.0f + offsetVertCH1;
                        g_waveCH1->y.upper = spanVertCH1 / 2.0f + offsetVertCH1;

                        PROC_SetSpan(SCOPE_CH1, spanVertCH1);
                        PROC_SetOffset(SCOPE_CH1, offsetVertCH1);
                    }
                } else if (g_activityRotCH1 == ROT_TRIGGER_VOLTAGE || g_activityRotCH1 == ROT_TRIGGER_VOLTAGE_FINE) {
                    // process trigger voltage adjustments
                    float step;
                    if (g_activityRotCH1 == ROT_TRIGGER_VOLTAGE) {
                        step = TRIGGER_STEP_COARSE;
                    } else if (g_activityRotCH1 == ROT_TRIGGER_VOLTAGE_FINE) {
                        step = TRIGGER_STEP_FINE;
                    }

                    bool isTriggerChanged = false;
                    if (curRotStateCH1.dir == ROT_DIR_CW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        g_triggerCH1.threshold = AddWithinMaximum_float(g_triggerCH1.threshold, step, TRIGGER_MAX);
                    } else if (curRotStateCH1.dir == ROT_DIR_CCW && prevRotStateCH1.dir == ROT_DIR_NONE) {
                        g_triggerCH1.threshold = SubWithinMinimum_float(g_triggerCH1.threshold, step, TRIGGER_MIN);
                    }

                    if (isTriggerChanged) {
                        g_waveCH1->trigger = g_triggerCH1.threshold;
                    }
                } else if (g_activityRotCH1 == ROT_CURSOR) {
                    while (1) {
                        // this shouldn't ever execute, cursor adjustments are designated to CH0
                    }
                }

                // process push button action
                if (curRotStateCH1.pressed && !prevRotStateCH1.pressed) {
                    if (g_activityRotCH1 != ROT_CURSOR) {
                        // go to next rot state
                        g_activityRotCH1 = (g_activityRotCH1 + 1) % ROT_LAST_NORM;
                    } else if (g_activityRotCH1 == ROT_CURSOR) {
                        // exit place cursor state
                        g_activityRotCH1 = ROT_HORIZONTAL_SCALE;
                    }

                    UpdateChannelLabel(g_guiLabelCH1, SCOPE_CH1, g_activityRotCH1);
                }

                isRotCH1Processed = true;
            }

            g_tickProcessControls = 0;
        } // end process controls
    }     // end while(1)
} // end CORE_Sys_Run()

static inline range_t TimeRangeFromFreq(float freq) {
    range_t ret;
    float   span = ADC_SAMPLES_PER_WAVEFORM / freq;
    ret.lower    = -1.0f * span / 2.0f;
    ret.upper    = span / 2.0f;
    return ret;
}

static void CORE_FrontEnd_Init(void) {
    PROC_SetAttenuation(SCOPE_CH0, ATTENUATION_X10);
    PROC_SetSpan(SCOPE_CH0, 10.0f);
    PROC_SetOffset(SCOPE_CH0, 0.0f);

    PROC_SetAttenuation(SCOPE_CH1, ATTENUATION_X10);
    PROC_SetSpan(SCOPE_CH1, 10.0f);
    PROC_SetOffset(SCOPE_CH1, 0.0f);

    return;
}

static gui_ret_t CORE_GUI_Init(void) {
    gui_ret_t ret;

    // create the root window
    point_t       pos_guiWindRoot = {.x = 0, .y = 0};
    rect_t        dim_guiWindRoot = {.w = screen.res.w - 1, .h = screen.res.h - 1};
    gui_window_t* guiWindRoot     = calloc(1, sizeof(gui_window_t));

    ret = GUI_Window_New(guiWindRoot, pos_guiWindRoot.x, pos_guiWindRoot.y, true, dim_guiWindRoot.w, dim_guiWindRoot.h);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiWindRoot = guiWindRoot;

    // create the graph
    point_t      pos_guiGraph = {.x = 50, .y = 50};
    rect_t       dim_guiGraph = {.w = screen.res.w - 2 * pos_guiGraph.x, .h = screen.res.h - 2 * 40};
    gui_graph_t* guiGraph     = calloc(1, sizeof(gui_graph_t));

    ret = GUI_Graph_New(guiGraph, pos_guiGraph.x, pos_guiGraph.y, true, dim_guiGraph.w, dim_guiGraph.h);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiGraph = guiGraph;

    // add the graph to the root window
    ret = GUI_Object_Add((gui_object_t*)guiGraph, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create the config button
    point_t       pos_guiButtonConf = {.x = pos_guiWindRoot.x + 4, .y = pos_guiWindRoot.y + 4};
    rect_t        dim_guiButtonConf = {.w = 7 * FONT_WIDTH + 4, FONT_HEIGHT + 4};
    gui_button_t* guiButtonConf     = calloc(1, sizeof(gui_button_t));

    ret = GUI_Button_New(
        guiButtonConf,
        pos_guiButtonConf.x,
        pos_guiButtonConf.y,
        true,
        dim_guiButtonConf.w,
        dim_guiButtonConf.h,
        "CONFIG");

    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // add the conf button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonConf, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create the cursor button
    point_t       pos_guiButtonCursor = {.x = pos_guiButtonConf.x + dim_guiButtonConf.w + 5, .y = pos_guiButtonConf.y};
    rect_t        dim_guiButtonCursor = {.w = 7 * FONT_WIDTH, .h = FONT_HEIGHT + 4};
    gui_button_t* guiButtonCursor     = calloc(1, sizeof(gui_button_t));

    ret = GUI_Button_New(
        guiButtonCursor,
        pos_guiButtonCursor.x,
        pos_guiButtonCursor.y,
        true,
        dim_guiButtonCursor.w,
        dim_guiButtonCursor.h,
        "CURSOR");

    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // add the add cursor button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonCursor, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create and add the CH0 trigger button
    rect_t  dim_guiButtonTriggerCH0   = {.w = 5 * FONT_WIDTH, .h = FONT_HEIGHT + 4};
    point_t pos_guiButtonTriggerCH0   = {.x = screen.res.w - dim_guiButtonTriggerCH0.w - 5, .y = pos_guiButtonCursor.y};
    gui_button_t* guiButtonTriggerCH0 = calloc(1, sizeof(gui_button_t));

    ret = GUI_Button_New(
        guiButtonTriggerCH0,
        pos_guiButtonTriggerCH0.x,
        pos_guiButtonTriggerCH0.y,
        true,
        dim_guiButtonTriggerCH0.w,
        dim_guiButtonTriggerCH0.h,
        "EDGE");

    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    g_guiButtonTriggerCH0 = guiButtonTriggerCH0;
    UpdateTriggerText(g_guiButtonTriggerCH0, g_triggerCH0);

    // add the add cursor button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonTriggerCH0, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create and add the CH0 label
    point_t      pos_guiLabelCH0 = {.x = pos_guiButtonTriggerCH0.x - 1 - 17 * FONT_WIDTH, .y = 6};
    gui_label_t* guiLabelCH0     = calloc(1, sizeof(gui_label_t));
    ret = GUI_Label_New(guiLabelCH0, pos_guiLabelCH0.x, pos_guiLabelCH0.y, true, "CH0: NULL", 1);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiLabelCH0 = guiLabelCH0;
    UpdateChannelLabel(g_guiLabelCH0, SCOPE_CH0, g_activityRotCH0);

    // add the CH0 label to the root window
    ret = GUI_Object_Add((gui_object_t*)guiLabelCH0, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create and add the CH1 trigger button
    rect_t  dim_guiButtonTriggerCH1 = {.w = dim_guiButtonTriggerCH0.w, .h = dim_guiButtonTriggerCH0.h};
    point_t pos_guiButtonTriggerCH1 = {
        .x = pos_guiButtonTriggerCH0.x,
        .y = pos_guiButtonTriggerCH0.y + dim_guiButtonTriggerCH0.h + 2};
    gui_button_t* guiButtonTriggerCH1 = calloc(1, sizeof(gui_button_t));

    ret = GUI_Button_New(
        guiButtonTriggerCH1,
        pos_guiButtonTriggerCH1.x,
        pos_guiButtonTriggerCH1.y,
        true,
        dim_guiButtonTriggerCH1.w,
        dim_guiButtonTriggerCH1.h,
        "EDGE");

    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    g_guiButtonTriggerCH1 = guiButtonTriggerCH1;
    UpdateTriggerText(g_guiButtonTriggerCH1, g_triggerCH1);

    // add the add cursor button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonTriggerCH1, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create and add the CH0 label
    point_t pos_guiLabelCH1  = {.x = pos_guiButtonTriggerCH1.x - 1 - 17 * FONT_WIDTH, .y = pos_guiButtonTriggerCH1.y + 2};
    gui_label_t* guiLabelCH1 = calloc(1, sizeof(gui_label_t));
    ret                      = GUI_Label_New(guiLabelCH1, pos_guiLabelCH1.x, pos_guiLabelCH1.y, true, "CH1: NULL", 1);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiLabelCH1 = guiLabelCH1;
    UpdateChannelLabel(g_guiLabelCH1, SCOPE_CH1, g_activityRotCH1);

    // add the CH0 label to the root window
    ret = GUI_Object_Add((gui_object_t*)guiLabelCH1, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // interactors
    gui_interactor_t* interConf       = calloc(1, sizeof(gui_interactor_t));
    gui_interactor_t* interCursor     = calloc(1, sizeof(gui_interactor_t));
    gui_interactor_t* interTriggerCH0 = calloc(1, sizeof(gui_interactor_t));
    gui_interactor_t* interTriggerCH1 = calloc(1, sizeof(gui_interactor_t));

    interConf->right    = interCursor;
    interConf->elem     = guiButtonConf;
    interConf->callback = &Callback_Conf;

    interCursor->left     = interConf;
    interCursor->right    = interTriggerCH0;
    interCursor->elem     = guiButtonCursor;
    interCursor->callback = &Callback_Cursor;

    interTriggerCH0->left     = interCursor;
    interTriggerCH0->down     = interTriggerCH1;
    interTriggerCH0->elem     = guiButtonTriggerCH0;
    interTriggerCH0->callback = &Callback_TriggerCH0;

    interTriggerCH1->left     = interCursor;
    interTriggerCH1->up       = interTriggerCH0;
    interTriggerCH1->elem     = guiButtonTriggerCH1;
    interTriggerCH1->callback = &Callback_TriggerCH1;

    g_interSel = interConf;
    GUI_Interactor_Select(g_interSel, JOY_DIR_NONE);

    // save the waveform pointers
    g_waveCH0 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH0);
    g_waveCH1 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH1);

    // initialize and save the theme pointer
    g_theme                = calloc(1, sizeof(gui_theme_t));
    g_theme->background    = color_black;
    g_theme->foreground    = color_white;
    g_theme->text          = color_white;
    g_theme->border        = color_white;
    g_theme->subtle        = color_gray;
    g_theme->accents[0]    = color_yellow;
    g_theme->accents[1]    = color_cyan;
    g_theme->accents[2]    = color_red;
    g_theme->accents[3]    = color_green;
    g_theme->selected      = color_white;
    g_theme->selected_text = color_black;
}
