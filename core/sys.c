#include "core/sys.h"

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

TIM_HandleTypeDef *g_pwm, *g_tick;

gui_waveform_t *g_waveCH0, *g_waveCH1;
gui_graph_t*    g_guiGraph;
gui_window_t*   g_guiWindSettings;
gui_theme_t*    g_theme;

uint32_t g_tickRender          = 0;
uint32_t g_tickSampleControls  = 0;
uint32_t g_tickProcessControls = 0;
uint32_t g_tickUpdateWaveforms = 0;
uint32_t g_tickSampleADC       = 0;

uint32_t g_tickSampleADCPrescaler = 0;

float       g_voltADCSamplesCH0[ADC_SAMPLES_PER_WAVEFORM];
size_t      g_indexADCSamplesCH0    = 0;
size_t      g_indexADCTriggerCH0    = 0;
size_t      g_lenADCAfterTriggerCH0 = 0;
adc_state_t g_stateCH0              = ADC_RUNNING;

float       g_voltADCSamplesCH1[ADC_SAMPLES_PER_WAVEFORM];
size_t      g_indexADCSamplesCH1    = 0;
size_t      g_indexADCTriggerCH1    = 0;
size_t      g_lenADCAfterTriggerCH1 = 0;
adc_state_t g_stateCH1              = ADC_RUNNING;

static void      CORE_FrontEnd_Init(void);
static gui_ret_t CORE_GUI_Init(void);

// Callback: timer has rolled over
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    if (htim == g_tick) {
        g_tickRender++;
        g_tickSampleControls++;
        g_tickProcessControls++;
        g_tickUpdateWaveforms++;
        g_tickADCPrescaler++;

        if (g_tickSampleADC > g_tickSampleADCPrescaler) {
            g_tickSampleADC = 0;

            g_voltADCSamplesCH0[g_indexADCSamplesCH0] = DRV_ADC_ReadVoltage(g_adc_ch0);
            g_indexADCSamplesCH0                      = (g_indexADCSamplesCH0 + 1) % lengthof(g_voltADCSamplesCH0);
        }
    }
}

void CORE_Sys_Init(
    ADC_HandleTypeDef* adc_ch0,
    ADC_HandleTypeDef* adc_ch1,
    DAC_HandleTypeDef* dac_vcal,
    SPI_HandleTypeDef* spi,
    TIM_HandleTypeDef* pwm,
    TIM_HandleTypeDef* tick) {
    // save pointers
    g_adc_ch0  = adc_ch0;
    g_adc_ch1  = adc_ch1;
    g_dac_vcal = dac_vcal;
    g_spi      = spi;
    g_pwm      = pwm;
    g_tick     = tick;

    // start backlight pwm
    HAL_TIM_Base_Start(g_pwm);
    HAL_TIM_PWM_Start(g_pwm, TIM_CHANNEL_1);

    // initialize the calibration dac
    HAL_DAC_Start(g_dac_vcal, DAC_CHANNEL_2);

    // initialize ticker and interrupt
    HAL_TIM_Base_Start_IT(g_tick);
    HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);

    // initialize the analog front end
    CORE_FrontEnd_Init();

    // initialize the GUI
    CORE_GUI_Init();

    return;
}

void CORE_Sys_Run(void) {
    const uint32_t tickRenderPeriod          = 0;
    const uint32_t tickSampleControlsPeriod  = 0;
    const uint32_t tickProcessControlsPeriod = 0;
    const uint32_t tickUpdateWaveformsPeriod = 0;

    while (1) {
        if (tickRender > tickRenderPeriod) {
            // GUI_Object_Render((gui_object_t*)guiWindRoot, &theme, (point_t){.x = 0, .y = 0});
            tickRender = 0;
        }
    }
}

static void CORE_FrontEnd_Init(void) {
    float           offset  = 1.0f;
    scope_channel_t channel = SCOPE_CH0;
    pga_gain_t      gain    = PGA_GAIN_1X;
    pga_channel_t   pregain = PGA_CH_1X;

    DRV_SPI_SetDacVoltage(g_spi, channel, offset);
    DRV_SPI_SetPgaChannel(g_spi, channel, pregain);
    DRV_SPI_SetPgaGain(g_spi, channel, gain);

    channel = SCOPE_CH1;
    DRV_SPI_SetDacVoltage(g_spi, channel, offset);
    DRV_SPI_SetPgaChannel(g_spi, channel, pregain);
    DRV_SPI_SetPgaGain(g_spi, channel, gain);
}

static gui_ret_t CORE_GUI_Init(void) {
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

    // (!!!) remove from init, belongs elsewhere
    g_waveCH0->x.lower = -100.0f;
    g_waveCH0->x.upper = 100.0f;
    g_waveCH0->y.lower = 0.0f;
    g_waveCH0->y.upper = 3.3f;

    g_waveCH1->x.lower = -100.0f;
    g_waveCH1->x.upper = 100.0f;
    g_waveCH1->y.lower = 0.0f;
    g_waveCH1->y.upper = 3.3f;
}
