#ifndef SYS_H
#define SYS_H

#include "main.h"

void CORE_SysInit(ADC_HandleTypeDef* adc_ch0, ADC_HandleTypeDef* adc_ch1, DAC_HandleTypeDef* dac_vcal, SPI_HandleTypeDef* spi, TIM_HandleTypeDef* pwm);

void CORE_SysRun(void);

#endif
