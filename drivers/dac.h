#ifndef DAC_H
#define DAC_H

#include "stm32h7xx_hal_dac.h"

void DRV_DAC_SetVoltage(DAC_HandleTypeDef *dac, float voltage);

#endif
