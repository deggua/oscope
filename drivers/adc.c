#include "main.h"
#include "stm32h7xx_hal_adc.h"

#include "drivers/adc.h"

float DRV_ADC_ReadVoltage(ADC_HandleTypeDef* adc) {
	HAL_ADC_Start(adc);
	HAL_ADC_PollForConversion(adc, HAL_MAX_DELAY);
	uint32_t voltBits = HAL_ADC_GetValue(adc);

	float voltMeasured = 3.3f*voltBits/255.0f;
}
