#ifndef SYS_H
#define SYS_H

#include "main.h"

void CORE_Sys_Init(
    ADC_HandleTypeDef* adc_ch0,
    ADC_HandleTypeDef* adc_ch1,
    DAC_HandleTypeDef* dac_vcal,
    SPI_HandleTypeDef* spi,
    TIM_HandleTypeDef* pwm);

void CORE_Sys_Run(void);

#endif
