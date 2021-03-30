#ifndef ADC_H
#define ADC_H

#include "stm32h7xx_hal_adc.h"
#include "utils/scope.h"

typedef enum { ADC_RET_SUCCESS, ADC_RET_UNINITIALIZED } adc_ret_t;

float DRV_ADC_ReadVoltage(scope_channel_t);

void DRV_ADC_Init(ADC_HandleTypeDef* adc_ch0, ADC_HandleTypeDef* adc_ch1);

#endif
