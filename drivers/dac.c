#include "drivers/dac.h"

#include <stdbool.h>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_dac.h"

DAC_HandleTypeDef* g_dac;
uint32_t           g_dacChannel;
bool               g_isDacInitialized = false;

void DRV_DAC_Init(DAC_HandleTypeDef* dac, uint32_t dac_channel) {
    g_dac        = dac;
    g_dacChannel = dac_channel;

    // initialize the calibration dac
    HAL_DAC_Start(g_dac, g_dacChannel);

    g_isDacInitialized = true;
    return;
}

dac_ret_t DRV_DAC_SetVoltage(float voltage) {
    if (g_isDacInitialized == false) {
        return DAC_RET_UNINITIALIZED;
    }

    float voltNormalize = voltage / 3.3f;

    if (voltNormalize > 1.0f) {
        voltNormalize = 1.0f;
    } else if (voltNormalize < 0.0f) {
        voltNormalize = 0.0f;
    }

    uint32_t voltBits = (uint32_t)(4095.0f * voltNormalize) & 0xFFF;

    HAL_DAC_SetValue(g_dac, g_dacChannel, DAC_ALIGN_12B_R, voltBits);

    return DAC_RET_SUCCESS;
}
