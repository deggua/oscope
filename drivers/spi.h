#ifndef SPI_H
#define SPI_H

#include "stm32h7xx_hal_spi.h"
#include "utils/scope.h"

typedef enum { PGA_CH_1X = 0, PGA_CH_12X = 1 } pga_channel_t;

typedef enum { SPI_RET_SUCCESS, SPI_RET_UNINITIALIZED } spi_ret_t;

typedef enum {
    PGA_GAIN_1X  = 0b000,
    PGA_GAIN_2X  = 0b001,
    PGA_GAIN_4X  = 0b010,
    PGA_GAIN_5X  = 0b011,
    PGA_GAIN_8X  = 0b100,
    PGA_GAIN_10X = 0b101,
    PGA_GAIN_16X = 0b110,
    PGA_GAIN_32X = 0b111
} pga_gain_t;

void DRV_SPI_SetDacVoltage(scope_channel_t channel, float voltage);

void DRV_SPI_SetPgaChannel(scope_channel_t channel, pga_channel_t pregain);

void DRV_SPI_SetPgaGain(scope_channel_t channel, pga_gain_t gain);

void DRV_SPI_Init(SPI_HandleTypeDef* spi);

#endif
