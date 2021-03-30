#include "drivers/adc.h"

#include <math.h>

#include "stm32h7xx_hal_adc.h"
#include "utils/scope.h"

ADC_HandleTypeDef *g_adcCH0, *g_adcCH1;
bool               g_isADCInitialized = false;

void DRV_ADC_Init(ADC_HandleTypeDef* adc_ch0, ADC_HandleTypeDef* adc_ch1) {
    g_adcCH0           = adc_ch0;
    g_adcCH1           = adc_ch1;
    g_isADCInitialized = true;
    return;
}

float DRV_ADC_ReadVoltage(scope_channel_t channel) {
    if (g_isADCInitialized == false) {
        return NAN;
    }

    uint32_t voltBits = HAL_ADC_GetValue(adc);
    if (channel == SCOPE_CH0) {
        HAL_ADC_Start(g_adcCH0);
        HAL_ADC_PollForConversion(g_adcCH0, HAL_MAX_DELAY);
        voltBits = HAL_ADC_GetValue(g_adcCH0);
    } else if (channel == SCOPE_CH1) {
        HAL_ADC_Start(g_adcCH1);
        HAL_ADC_PollForConversion(g_adcCH1, HAL_MAX_DELAY);
        voltBits = HAL_ADC_GetValue(g_adcCH1);
    }

    float voltMeasured = 3.28f * voltBits / 255.0f;
    return voltMeasured;
}
