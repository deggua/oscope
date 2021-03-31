#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdint.h>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"

typedef enum { BACKLIGHT_RET_SUCCESS, BACKLIGHT_RET_UNINITIALIZED } backlight_ret_t;

void DRV_Backlight_Init(TIM_HandleTypeDef* pwm, uint32_t channel, uint32_t period);

backlight_ret_t DRV_Backlight_SetBrightness(float norm_brightness);

#endif
