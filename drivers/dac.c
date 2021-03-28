#include "main.h"
#include "stm32h7xx_hal_dac.h"

void DRV_DAC_SetVoltage(DAC_HandleTypeDef *dac, float voltage) {
	float voltNormalize = voltage/3.3f;

	if (voltNormalize > 1.0f) {
		voltNormalize = 1.0f;
	} else if (voltNormalize < 0.0f) {
		voltNormalize = 0.0f;
	}

	uint32_t voltBits = (uint32_t)(4095.0f * voltNormalize) & 0xFFF;

	HAL_DAC_SetValue(dac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, voltBits);

	return;
}
