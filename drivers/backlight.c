#include "drivers/backlight.h"

#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"

TIM_HandleTypeDef* g_pwm;
uint32_t           g_timChannel, g_timPeriod;
bool               g_isPWMInitialized = false;

void DRV_Backlight_Init(TIM_HandleTypeDef* pwm, uint32_t channel, uint32_t period) {
    g_pwm        = pwm;
    g_timChannel = channel;
    g_timPeriod  = period;

    // start backlight pwm
    HAL_TIM_Base_Start(g_pwm);
    HAL_TIM_PWM_Start(g_pwm, g_timChannel);

    g_isPWMInitialized = true;
    return;
}

backlight_ret_t DRV_Backlight_SetBrightness(float norm_brightness) {
    if (g_isPWMInitialized == false) {
        return BACKLIGHT_RET_UNINITIALIZED;
    }

    if (norm_brightness > 1.0f) {
        norm_brightness = 1.0f;
    } else if (norm_brightness < 0.0f) {
        norm_brightness = 0.0f;
    }

    HAL_TIM_Base_Stop(g_pwm);
    HAL_TIM_PWM_Stop(g_pwm, g_timChannel);

    uint32_t           dutyCycle = (uint32_t)(norm_brightness * (float)g_timPeriod);
    TIM_OC_InitTypeDef sConfigOC = {0};

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = dutyCycle;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(g_pwm, &sConfigOC, g_timChannel) != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(g_pwm);

    HAL_TIM_Base_Start(g_pwm);
    HAL_TIM_PWM_Start(g_pwm, g_timChannel);

    return BACKLIGHT_RET_SUCCESS;
}
