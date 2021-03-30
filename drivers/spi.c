#include "drivers/spi.h"

#include <stdbool.h>

#include "main.h"
#include "stm32h7xx_hal_spi.h"
#include "utils/scope.h"

SPI_HandleTypeDef* g_spi;
bool               g_isSpiInitialized = false;

void DRV_SPI_Init(SPI_HandleTypeDef* spi) {
    g_spi              = spi;
    g_isSpiInitialized = true;
    return;
}

spi_ret_t DRV_SPI_SetDacVoltage(scope_channel_t channel, float voltage) {
    if (g_isSpiInitialized == false) {
        return SPI_RET_UNINITIALIZED;
    }

    uint16_t cmd = 0x0000;

    float voltNormalized = voltage / 3.3f;
    if (voltNormalized > 1.0f) {
        voltNormalized = 1.0f;
    } else if (voltNormalized < 0.0f) {
        voltNormalized = 0.0f;
    }

    uint16_t voltBits = (uint16_t)(voltNormalized * 4095.0f);
    cmd               = (voltBits & 0xFFF);

    if (channel == SCOPE_CH0) {
        cmd &= ~(1 << 15);
    } else if (channel == SCOPE_CH1) {
        cmd |= (1 << 15);
    }

    // buf = 0
    // gain = 1x
    cmd |= (1 << 13);
    // ouput = on
    cmd |= (1 << 12);

    HAL_GPIO_WritePin(CSn_DAC_GPIO_Port, CSn_DAC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(g_spi, (uint8_t*)&cmd, 1, 100);
    HAL_GPIO_WritePin(CSn_DAC_GPIO_Port, CSn_DAC_Pin, GPIO_PIN_SET);

    return SPI_RET_SUCCESS;
}

spi_ret_t DRV_SPI_SetPgaChannel(scope_channel_t channel, pga_channel_t pregain) {
    if (g_isSpiInitialized == false) {
        return SPI_RET_UNINITIALIZED;
    }

    uint16_t cmd = 0x0000;

    // set channel
    cmd = (0b111 & pregain);

    // command = write to register
    cmd |= (0b010 << 13);
    // set channel register
    cmd |= (1 << 8);

    if (channel == SCOPE_CH0) {
        HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(g_spi, (uint8_t*)&cmd, 1, 100);
        HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_SET);
    } else if (channel == SCOPE_CH1) {
        HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(g_spi, (uint8_t*)&cmd, 1, 100);
        HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_SET);
    }

    return SPI_RET_SUCCESS;
}

spi_ret_t DRV_SPI_SetPgaGain(scope_channel_t channel, pga_gain_t gain) {
    if (g_isSpiInitialized == false) {
        return SPI_RET_UNINITIALIZED;
    }

    uint16_t cmd = 0x0000;

    cmd = (0b111 & gain);
    // command = write to register
    cmd |= (0b010 << 13);
    // set gain register
    // set gain

    if (channel == SCOPE_CH0) {
        HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(g_spi, (uint8_t*)&cmd, 1, 100);
        HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_SET);
    } else if (channel == SCOPE_CH1) {
        HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(g_spi, (uint8_t*)&cmd, 1, 100);
        HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_SET);
    }

    return SPI_RET_SUCCESS;
}
