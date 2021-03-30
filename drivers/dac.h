#ifndef DAC_H
#define DAC_H

#include <stdint.h>

#include "stm32h7xx_hal_dac.h"

typedef enum { DAC_RET_SUCCESS, DAC_RET_UNINITIALIZED } dac_ret_t;

void DRV_DAC_SetVoltage(float voltage);

void DRV_DAC_Init(DAC_HandleTypeDef* dac, uint32_t dac_channel);

#endif
