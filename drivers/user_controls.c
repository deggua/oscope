#include "drivers/user_controls.h"

#include <stdbool.h>

#include "main.h"
#include "utils/scope.h"

joy_state_t DRV_Joy_GetState(void) {
    joy_state_t ret;

    if (HAL_GPIO_ReadPin(JoyDown_GPIO_Port, JoyDown_Pin) == GPIO_PIN_RESET) {
        ret.dir = JOY_DIR_DOWN;
    } else if (HAL_GPIO_ReadPin(JoyUp_GPIO_Port, JoyUp_Pin) == GPIO_PIN_RESET) {
        ret.dir = JOY_DIR_UP;
    } else if (HAL_GPIO_ReadPin(JoyLeft_GPIO_Port, JoyLeft_Pin) == GPIO_PIN_RESET) {
        ret.dir = JOY_DIR_LEFT;
    } else if (HAL_GPIO_ReadPin(JoyRight_GPIO_Port, JoyRight_Pin) == GPIO_PIN_RESET) {
        ret.dir = JOY_DIR_RIGHT;
    } else {
        ret.dir = JOY_DIR_NONE;
    }

    if (HAL_GPIO_ReadPin(JoyPB_GPIO_Port, JoyPB_Pin) == GPIO_PIN_RESET) {
        ret.pressed = true;
    } else {
        ret.pressed = false;
    }

    return ret;
}

rot_state_t DRV_Rot_GetState(scope_channel_t channel) {
    rot_state_t ret;

    if (channel == SCOPE_CH0) {
        static bool prevA, prevB;
        bool        curA, curB;
        curA = (HAL_GPIO_ReadPin(RotA_CH0_GPIO_Port, RotA_CH0_Pin) == GPIO_PIN_RESET);
        curB = (HAL_GPIO_ReadPin(RotB_CH0_GPIO_Port, RotB_CH0_Pin) == GPIO_PIN_RESET);

        if (curA == true && prevB == false && curB == true) {
            ret.dir = ROT_DIR_CW;
        } else if (curB == true && prevA == false && curA == true) {
            ret.dir = ROT_DIR_CCW;
        } else {
            ret.dir = ROT_DIR_NONE;
        }

        if (HAL_GPIO_ReadPin(RotPB_CH0_GPIO_Port, RotPB_CH0_Pin) == GPIO_PIN_RESET) {
            ret.pressed = true;
        } else {
            ret.pressed = false;
        }

        prevA = curA;
        prevB = curB;

    } else if (channel == SCOPE_CH1) {
        static bool prevA, prevB;
        bool        curA, curB;
        curA = (HAL_GPIO_ReadPin(RotA_CH1_GPIO_Port, RotA_CH1_Pin) == GPIO_PIN_SET);
        curB = (HAL_GPIO_ReadPin(RotB_CH1_GPIO_Port, RotB_CH1_Pin) == GPIO_PIN_SET);

        if (curA == true && prevB == false && curB == true) {
            ret.dir = ROT_DIR_CW;
        } else if (curB == true && prevA == false && curA == true) {
            ret.dir = ROT_DIR_CCW;
        } else {
            ret.dir = ROT_DIR_NONE;
        }

        if (HAL_GPIO_ReadPin(RotPB_CH1_GPIO_Port, RotPB_CH1_Pin) == GPIO_PIN_RESET) {
            ret.pressed = true;
        } else {
            ret.pressed = false;
        }

        prevA = curA;
        prevB = curB;
    } else {
        ret.dir     = ROT_DIR_NONE;
        ret.pressed = false;
    }

    return ret;
}
