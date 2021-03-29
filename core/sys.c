#include "core/sys.h"

#include "core/init.h"
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

ADC_HandleTypeDef* g_adc_ch0;
ADC_HandleTypeDef* g_adc_ch1;

DAC_HandleTypeDef* g_dac_vcal;

SPI_HandleTypeDef* g_spi;

TIM_HandleTypeDef* g_pwm;

gui_waveform_t *g_waveCH0, *g_waveCH1;
gui_graph_t*    g_guiGraph;
gui_window_t*   g_guiWindSettings;
gui_theme_t*    g_theme;

void CORE_Sys_Init(
    ADC_HandleTypeDef* adc_ch0,
    ADC_HandleTypeDef* adc_ch1,
    DAC_HandleTypeDef* dac_vcal,
    SPI_HandleTypeDef* spi,
    TIM_HandleTypeDef* pwm) {
    // start backlight pwm
    HAL_TIM_Base_Start(pwm);
    HAL_TIM_PWM_Start(pwm, TIM_CHANNEL_1);

    // initialize the calibration dac
    HAL_DAC_Start(dac_vcal, DAC_CHANNEL_2);

    // initialize the analog front end

    // initialize the GUI
    CORE_GUI_Init();
}

void CORE_FrontEnd_Init(void) {
    float           offset  = 1.0f;
    scope_channel_t channel = SCOPE_CH0;
    pga_gain_t      gain    = PGA_GAIN_1X;
    pga_channel_t   pregain = PGA_CH_1X;

    DRV_SPI_SetDacVoltage(&hspi4, channel, offset);
    DRV_SPI_SetPgaChannel(&hspi4, channel, pregain);
    DRV_SPI_SetPgaGain(&hspi4, channel, gain);

    channel = SCOPE_CH1;
    DRV_SPI_SetDacVoltage(&hspi4, channel, offset);
    DRV_SPI_SetPgaChannel(&hspi4, channel, pregain);
    DRV_SPI_SetPgaGain(&hspi4, channel, gain);
}

gui_ret_t CORE_GUI_Init(void) {
    gui_ret_t ret;

    // create the root window
    gui_window_t* guiWindRoot = calloc(1, sizeof(gui_window_t));
    ret                       = GUI_Window_New(guiWindRoot, 0, 0, true, screen.res.w - 1, screen.res.h - 1);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

    // create the graph
    gui_graph_t* guiGraph = calloc(1, sizeof(gui_graph_t));
    ret                   = GUI_Graph_New(guiGraph, 30, 20, true, screen.res.w - 60, screen.res.h - 40);
    if (ret != GUI_RET_SUCCESS) {
        return ret;
    }

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

    // save the waveform pointers
    g_waveCH0 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH0);
    g_waveCH1 = GUI_Graph_GetWaveform(guiGraph, SCOPE_CH1);

    // initialize and save the theme pointer
    g_theme            = calloc(1, sizeof(gui_theme_t));
    g_theme.background = color_black;
    g_theme.foreground = color_white;
    g_theme.text       = color_white;
    g_theme.border     = color_white;
    g_theme.subtle     = color_gray;
    g_theme.accents[0] = color_yellow;
    g_theme.accents[1] = color_cyan;
    g_theme.accents[2] = color_red;
    g_theme.accents[3] = color_green;

    // (!!!) remove from init, belongs elsewhere
    waveCH0->x.lower = -100.0f;
    waveCH0->x.upper = 100.0f;
    waveCH0->y.lower = 0.0f;
    waveCH0->y.upper = 3.3f;

    waveCH1->x.lower = -100.0f;
    waveCH1->x.upper = 100.0f;
    waveCH1->y.lower = 0.0f;
    waveCH1->y.upper = 3.3f;
}

void CORE_Sys_Run(void) {
    uint32_t tickRender = 0;
    while (1) {
        GUI_Object_Render((gui_object_t*)guiWindRoot, &theme, (point_t){.x = 0, .y = 0});
    }
}