#include "core/sys.h"

#include <stdbool.h>
#include <stdlib.h>

#include "display/display.h"
#include "drivers/adc.h"
#include "drivers/dac.h"
#include "drivers/spi.h"
#include "drivers/user_controls.h"
#include "gui/elements/gui_button.h"
#include "gui/elements/gui_graph.h"
#include "gui/elements/gui_window.h"
#include "gui/gui.h"
#include "gui/gui_base.h"
#include "gui/gui_interactor.h"
#include "processing/control.h"

#define ADC_FREQ (50000.0f)

#define HORIZONTAL_SCALE_STEP_COARSE 20
#define HORIZONTAL_SCALE_STEP_FINE   1
#define VERTICAL_SCALE_STEP_COARSE   1
#define VERTICAL_SCALE_STEP_FINE     0.1
#define VERTICAL_SPAN_MIN            0.1f
#define VERTICAL_SPAN_MAX            33.0f
#define OFFSET_STEP_FINE             0.1f;
#define OFFSET_STEP_COARSE           0.5f;
#define OFFSET_MIN                   0.0f;

// ticker reference
TIM_HandleTypeDef* g_tick;

// GUI references
gui_waveform_t *  g_waveCH0, *g_waveCH1;
gui_graph_t*      g_guiGraph;
gui_window_t*     g_guiWindSettings;
gui_window_t* g_guiWindRoot;
gui_theme_t*      g_theme;
gui_interactor_t* g_interSel;

// ticker counters
volatile uint32_t g_tickRender          = 0;
volatile uint32_t g_tickSampleControls  = 0;
volatile uint32_t g_tickProcessControls = 0;
volatile uint32_t g_tickSampleADC       = 0;

// adc ticker prescaler
volatile uint32_t g_tickSampleADCPrescaler = 0;

// CH0 globals
volatile float       g_voltADCSamplesCH0[ADC_SAMPLES_PER_WAVEFORM];
volatile size_t      g_indexADCSamplesCH0    = 0;
volatile size_t      g_lenADCAfterTriggerCH0 = 0;
volatile size_t g_lenADCBeforeTriggerCH0 = 0;
volatile adc_state_t g_stateCH0              = ADC_RUNNING;
volatile trigger_t   g_triggerCH0 = {.edge = EDGE_RISING, .threshold = 0.0f};

// CH1 globals
volatile float       g_voltADCSamplesCH1[ADC_SAMPLES_PER_WAVEFORM];
volatile size_t      g_indexADCSamplesCH1    = 0;
volatile size_t g_lenADCBeforeTriggerCH1 = 0;
volatile size_t      g_lenADCAfterTriggerCH1 = 0;
volatile adc_state_t g_stateCH1              = ADC_RUNNING;
volatile trigger_t   g_triggerCH1 = {.edge = EDGE_RISING, .threshold = 0.0f};

volatile uint32_t g_DEBUG_idxCenter = 0;

static void      CORE_FrontEnd_Init(void);
static gui_ret_t CORE_GUI_Init(void);
static void      Callback_Conf(void);
static void      Callback_AddCursor(void);
static range_t   TimeRangeFromFreq(float freq);

static bool isTriggerTripped(float prevSample, float curSample, trigger_t* trig) {
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

static void CopyCircularWaveform(float* dst, float* src, size_t idx_first, size_t len) {
    size_t indexCur = idx_first;
    for (int32_t ii = 0; ii < len; ii++) {
        dst[ii]  = src[indexCur];
        indexCur = (indexCur + 1) % len;
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
                g_voltADCSamplesCH0[g_indexADCSamplesCH0] = PROC_SampleVoltageCalibrated(SCOPE_CH0);

                if (g_lenADCBeforeTriggerCH0 > 32) {
                    float  curSampleCH0 = g_voltADCSamplesCH0[g_indexADCSamplesCH0];
                    size_t prevIndexCH0 =
                        (g_indexADCSamplesCH0 + lengthof(g_voltADCSamplesCH0) - 1) % lengthof(g_voltADCSamplesCH0);
                    float prevSampleCH0 = g_voltADCSamplesCH0[prevIndexCH0];

                    // process trigger for CH0
                    if (isTriggerTripped(prevSampleCH0, curSampleCH0, &g_triggerCH0)) {
                        g_lenADCAfterTriggerCH0 = 0;
                        g_lenADCBeforeTriggerCH0 = 0;
                        g_stateCH0 = ADC_TRIGGERED;
                        g_DEBUG_idxCenter = g_indexADCSamplesCH0;
                    }
                }

                g_lenADCBeforeTriggerCH0++;
                g_indexADCSamplesCH0 = (g_indexADCSamplesCH0 + 1) % lengthof(g_voltADCSamplesCH0);
            } else if (g_stateCH0 == ADC_TRIGGERED && g_lenADCAfterTriggerCH0 < ADC_SAMPLES_PER_WAVEFORM / 2 - 1) {
                // store ADC sample
                g_voltADCSamplesCH0[g_indexADCSamplesCH0] = PROC_SampleVoltageCalibrated(SCOPE_CH0);
                g_lenADCAfterTriggerCH0++;
                g_indexADCSamplesCH0 = (g_indexADCSamplesCH0 + 1) % lengthof(g_voltADCSamplesCH0);
            } else {
                // mark that a full waveform is available
                g_stateCH0              = ADC_AVAILABLE;
            }

            // process adc state for CH1
            if (g_stateCH1 == ADC_RUNNING) {
                // store ADC sample
                g_voltADCSamplesCH1[g_indexADCSamplesCH1] = PROC_SampleVoltageCalibrated(SCOPE_CH1);

                float  curSampleCH1 = g_voltADCSamplesCH1[g_indexADCSamplesCH1];
                size_t prevIndexCH1 =
                    (g_indexADCSamplesCH1 + lengthof(g_voltADCSamplesCH1) - 1) % lengthof(g_voltADCSamplesCH1);
                float prevSampleCH1 = g_voltADCSamplesCH1[prevIndexCH1];

                // process trigger for CH1
                if (isTriggerTripped(prevSampleCH1, curSampleCH1, &g_triggerCH1)) {
                    g_lenADCAfterTriggerCH1 = 0;
                    g_stateCH1 = ADC_TRIGGERED;
                }

                g_indexADCSamplesCH1 = (g_indexADCSamplesCH1 + 1) % lengthof(g_voltADCSamplesCH1);
            } else if (g_stateCH1 == ADC_TRIGGERED && g_lenADCAfterTriggerCH1 < ADC_SAMPLES_PER_WAVEFORM / 2 - 1) {
                // store ADC sample
                g_voltADCSamplesCH1[g_indexADCSamplesCH1] = PROC_SampleVoltageCalibrated(SCOPE_CH1);
                g_lenADCAfterTriggerCH1++;
                g_indexADCSamplesCH1 = (g_indexADCSamplesCH1 + 1) % lengthof(g_voltADCSamplesCH1);
            } else {
                // mark that a full waveform is available
                g_stateCH1              = ADC_AVAILABLE;

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
    const uint32_t tickRenderPeriod          = ADC_FREQ / 30;  // 30 Hz redraw
    const uint32_t tickSampleControlsPeriod  = ADC_FREQ / 1000;    // 1000 Hz sampling
    const uint32_t tickProcessControlsPeriod = ADC_FREQ / 4; // 4 Hz execution of controls

    joy_state_t curJoyState = {0};
    bool        isJoyDirNew = false;
    bool        isJoyPBNew  = false;

    rot_state_t curRotStateCH0 = {0};
    bool        isRotCH0DirNew = false;
    bool        isRotCH0PBNew  = false;
    scope_rot_t activityRotCH0 = ROT_HORIZONTAL_SCALE;

    rot_state_t curRotStateCH1 = {0};
    bool        isRotCH1DirNew = false;
    bool        isRotCH1PBNew  = false;
    scope_rot_t activityRotCH1 = ROT_HORIZONTAL_SCALE;

    float spanVertCH0   = 10.0f;
    float offsetVertCH0 = 0.0f;
    float spanVertCH1   = 10.0f;
    float offsetVertCH1 = 0.0f;

    range_t newRange   = TimeRangeFromFreq(ADC_FREQ / g_tickSampleADCPrescaler);
    g_waveCH0->x.lower = newRange.lower;
    g_waveCH0->x.upper = newRange.upper;
    g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f;
    g_waveCH0->y.upper = spanVertCH0 / 2.0f;

    PROC_SetSpan(SCOPE_CH0, spanVertCH0);
    PROC_SetOffset(SCOPE_CH0, offsetVertCH0);

    while (1) {
        if (g_tickRender > tickRenderPeriod) {
            // update waveforms
            if (g_stateCH0 == ADC_AVAILABLE) {
                CopyCircularWaveform(
                    g_waveCH0->samples,
                    g_voltADCSamplesCH0,
                    g_indexADCSamplesCH0,
                    ADC_SAMPLES_PER_WAVEFORM);
                g_stateCH0 = ADC_RUNNING;
            }

            if (g_stateCH1 == ADC_AVAILABLE) {
                CopyCircularWaveform(
                    g_waveCH1->samples,
                    g_voltADCSamplesCH1,
                    g_indexADCSamplesCH1,
                    ADC_SAMPLES_PER_WAVEFORM);
                g_stateCH0 = ADC_RUNNING;
            }

            // Redraw the GUI
            GUI_Object_Render((gui_object_t*)g_guiWindRoot, g_theme, (point_t){.x = 0, .y = 0});
            g_tickRender = 0;
        }

        if (g_tickSampleControls > tickSampleControlsPeriod) {
            // sample user controls
            joy_state_t tempJoyState = DRV_Joy_GetState();
            if (isJoyDirNew == false && tempJoyState.dir != JOY_DIR_NONE) {
                curJoyState.dir = tempJoyState.dir;
                isJoyDirNew     = true;
            }

            if (isJoyPBNew == false && tempJoyState.pressed == true) {
                curJoyState.pressed = true;
                isJoyPBNew          = true;
            }

            rot_state_t tempRotStateCH0 = DRV_Rot_GetState(SCOPE_CH0);
            if (isRotCH0DirNew == false && tempRotStateCH0.dir != ROT_DIR_NONE) {
                curRotStateCH0.dir = tempRotStateCH0.dir;
                isRotCH0DirNew     = true;
            }

            if (isRotCH0PBNew == false && tempRotStateCH0.pressed == true) {
                curRotStateCH0.pressed = true;
                isRotCH0PBNew          = true;
            }

            rot_state_t tempRotStateCH1 = DRV_Rot_GetState(SCOPE_CH1);
            if (isRotCH1DirNew == false && tempRotStateCH1.dir != ROT_DIR_NONE) {
                curRotStateCH1.dir = tempRotStateCH1.dir;
                isRotCH1DirNew     = true;
            }

            if (isRotCH1PBNew == false && tempRotStateCH1.pressed == true) {
                curRotStateCH1.pressed = true;
                isRotCH1PBNew          = true;
            }

            g_tickSampleControls = 0;
        }

        if (g_tickProcessControls > tickProcessControlsPeriod) {
            // execute based on user controls values
            if (isJoyDirNew) {
                isJoyDirNew = false;
                // update interactor
                g_interSel = GUI_Interactor_Select(g_interSel, curJoyState.dir);
            }

            if (isJoyPBNew) {
                isJoyPBNew = false;
                // call interactor callback
                GUI_Interactor_Execute(g_interSel);
            }

            if (isRotCH0DirNew) {
                isRotCH0DirNew = false;
                if (activityRotCH0 == ROT_HORIZONTAL_SCALE) {
                    // coarse horizontal adjustment of CH0
                    if (curRotStateCH0.dir == ROT_DIR_CW) {
                        if (g_tickSampleADCPrescaler < UINT32_MAX - HORIZONTAL_SCALE_STEP_COARSE) {
                            g_tickSampleADCPrescaler += HORIZONTAL_SCALE_STEP_COARSE;
                        } else {
                            g_tickSampleADCPrescaler = UINT32_MAX;
                        }

                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / g_tickSampleADCPrescaler);
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW) {
                        if (g_tickSampleADCPrescaler > HORIZONTAL_SCALE_STEP_COARSE) {
                            g_tickSampleADCPrescaler -= HORIZONTAL_SCALE_STEP_COARSE;
                        } else {
                            g_tickSampleADCPrescaler = 0;
                        }

                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / g_tickSampleADCPrescaler);
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    }

                } else if (activityRotCH0 == ROT_HORIZONTAL_SCALE_FINE) {
                    // fine horizontal adjustment of CH0
                    if (curRotStateCH0.dir == ROT_DIR_CW) {
                        if (g_tickSampleADCPrescaler < UINT32_MAX - HORIZONTAL_SCALE_STEP_FINE) {
                            g_tickSampleADCPrescaler += HORIZONTAL_SCALE_STEP_FINE;
                        } else {
                            g_tickSampleADCPrescaler = UINT32_MAX;
                        }

                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / g_tickSampleADCPrescaler);
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW) {
                        if (g_tickSampleADCPrescaler > HORIZONTAL_SCALE_STEP_FINE) {
                            g_tickSampleADCPrescaler -= HORIZONTAL_SCALE_STEP_FINE;
                        } else {
                            g_tickSampleADCPrescaler = 0;
                        }

                        range_t newRange   = TimeRangeFromFreq(ADC_FREQ / g_tickSampleADCPrescaler);
                        g_waveCH0->x.lower = newRange.lower;
                        g_waveCH0->x.upper = newRange.upper;
                    }

                } else if (activityRotCH0 == ROT_VERTICAL_SCALE) {
                    // coarse vertical adjustment of CH0
                    if (curRotStateCH0.dir == ROT_DIR_CW) {
                        if (spanVertCH0 < VERTICAL_SPAN_MAX - VERTICAL_SCALE_STEP_COARSE) {
                            spanVertCH0 += VERTICAL_SCALE_STEP_COARSE;
                        } else {
                            spanVertCH0 = VERTICAL_SPAN_MAX;
                        }

                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW) {
                        if (spanVertCH0 > VERTICAL_SPAN_MIN + VERTICAL_SCALE_STEP_COARSE) {
                            spanVertCH0 -= VERTICAL_SCALE_STEP_COARSE;
                        } else {
                            spanVertCH0 = VERTICAL_SPAN_MIN;
                        }

                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    }
                } else if (activityRotCH0 == ROT_VERTICAL_SCALE_FINE) {
                    // fine vertical adjustment of CH0
                    if (curRotStateCH0.dir == ROT_DIR_CW) {
                        if (spanVertCH0 < VERTICAL_SPAN_MAX - VERTICAL_SCALE_STEP_FINE) {
                            spanVertCH0 += VERTICAL_SCALE_STEP_FINE;
                        } else {
                            spanVertCH0 = VERTICAL_SPAN_MAX;
                        }

                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    } else if (curRotStateCH0.dir == ROT_DIR_CCW) {
                        if (spanVertCH0 > VERTICAL_SPAN_MIN + VERTICAL_SCALE_STEP_FINE) {
                            spanVertCH0 -= VERTICAL_SCALE_STEP_FINE;
                        } else {
                            spanVertCH0 = VERTICAL_SPAN_MIN;
                        }

                        g_waveCH0->y.lower = -1.0f * spanVertCH0 / 2.0f;
                        g_waveCH0->y.upper = spanVertCH0 / 2.0f;

                        PROC_SetSpan(SCOPE_CH0, spanVertCH0);
                        PROC_SetOffset(SCOPE_CH0, offsetVertCH0);
                    }
                }
                // execute CH0 rot action based on rot CH0 state
            }

            if (isRotCH0PBNew) {
                isRotCH0PBNew = false;
                // execute CH0 pb action based on rot CH0 state
                if (activityRotCH0 == ROT_CURSOR || activityRotCH0 == ROT_CURSOR_FINE) {
                    activityRotCH0 = ROT_HORIZONTAL_SCALE;
                } else {
                    activityRotCH0 = (activityRotCH0 + 1) % ROT_LAST_NORM;
                }
            }

            if (isRotCH1DirNew) {
                isRotCH1DirNew = false;
                // execute CH0 rot action based on rot CH0 state
            }

            if (isRotCH1PBNew) {
                isRotCH1PBNew = false;
                // execute CH0 pb action based on rot CH0 state
                if (activityRotCH0 == ROT_CURSOR || activityRotCH0 == ROT_CURSOR_FINE) {
                    activityRotCH0 = ROT_HORIZONTAL_SCALE;
                } else {
                    activityRotCH0 = (activityRotCH0 + 1) % ROT_LAST_NORM;
                }
            }

            g_tickProcessControls = 0;
        }
    }
}

range_t TimeRangeFromFreq(float freq) {
    range_t ret;
    float   span = ADC_SAMPLES_PER_WAVEFORM * (1 / freq);
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
    gui_window_t* guiWindRoot = calloc(1, sizeof(gui_window_t));
    ret                       = GUI_Window_New(guiWindRoot, 0, 0, true, screen.res.w - 1, screen.res.h - 1);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiWindRoot = guiWindRoot;

    // create the graph
    gui_graph_t* guiGraph = calloc(1, sizeof(gui_graph_t));
    ret                   = GUI_Graph_New(guiGraph, 30, 20, true, screen.res.w - 60, screen.res.h - 40);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }
    g_guiGraph = guiGraph;

    // add the graph to the root window
    ret = GUI_Object_Add((gui_object_t*)guiGraph, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // creat the conf button
    gui_button_t* guiButtonConf = calloc(1, sizeof(gui_button_t));
    ret = GUI_Button_New(guiButtonConf, 30, 4, true, 5 * FONT_WIDTH + 4, FONT_HEIGHT + 4, "CONF");
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // add the conf button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonConf, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create the add cursor button
    gui_button_t* guiButtonAddCursor = calloc(1, sizeof(gui_button_t));
    ret                              = GUI_Button_New(
        guiButtonAddCursor,
        30 + screen.res.w - 60 - (7 * FONT_WIDTH + 4),
        4,
        true,
        7 * FONT_WIDTH + 4,
        FONT_HEIGHT + 4,
        "CURSOR");
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // add the add cursor button to the root window
    ret = GUI_Object_Add((gui_object_t*)guiButtonAddCursor, (gui_object_t*)guiWindRoot);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // interactors
    gui_interactor_t* interConf      = calloc(1, sizeof(gui_interactor_t));
    gui_interactor_t* interAddCursor = calloc(1, sizeof(gui_interactor_t));

    interConf->right    = interAddCursor;
    interConf->elem     = guiButtonConf;
    interConf->callback = &Callback_Conf;

    interAddCursor->left     = interConf;
    interAddCursor->elem     = guiButtonAddCursor;
    interAddCursor->callback = &Callback_AddCursor;

    g_interSel = interConf;
    GUI_Interactor_Select(g_interSel, JOY_DIR_NONE);

    // save the waveform pointers
    g_waveCH0 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH0);
    g_waveCH1 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH1);

    // initialize and save the theme pointer
    g_theme             = calloc(1, sizeof(gui_theme_t));
    g_theme->background = color_black;
    g_theme->foreground = color_white;
    g_theme->text       = color_white;
    g_theme->border     = color_white;
    g_theme->subtle     = color_gray;
    g_theme->accents[0] = color_yellow;
    g_theme->accents[1] = color_cyan;
    g_theme->accents[2] = color_red;
    g_theme->accents[3] = color_green;
    g_theme->selected = color_white;
    g_theme->selected_text = color_black;
}

void Callback_Conf(void) {}

void Callback_AddCursor(void) {}
