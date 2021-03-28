#include "main.h"

#include "drivers/spi.h"
#include "utils/scope.h"

void DRV_SPI_SetDacVoltage(SPI_HandleTypeDef* spi, scope_channel_t channel, float voltage) {
	uint16_t cmd = 0x0000;

	float voltNormalized = voltage/3.3f;
	if (voltNormalized > 1.0f) {
		voltNormalized = 1.0f;
	} else if (voltNormalized < 0.0f) {
		voltNormalized = 0.0f;
	}

	uint16_t voltBits = (uint16_t)(voltNormalized * 4095.0f);
	cmd = (voltBits & 0xFFF);

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
	// (!!!) might be little endian which might result in the bytes being sent in reverse order,
	// if it doesn't work try swapping the bytes first
	HAL_SPI_Transmit(spi, (uint8_t*)&cmd, 1, 100);
	HAL_GPIO_WritePin(CSn_DAC_GPIO_Port, CSn_DAC_Pin, GPIO_PIN_SET);

	return;
}

void DRV_SPI_SetPgaChannel(SPI_HandleTypeDef* spi, scope_channel_t channel, pga_channel_t pregain) {
	uint16_t cmd = 0x0000;

	// set channel
	cmd = (0b111 & pregain);

	// command = write to register
	cmd |= (0b010 << 13);
	// set channel register
	cmd |= (1 << 8);


	if (channel == SCOPE_CH0) {
		HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(spi, (uint8_t*)&cmd, 1, 100);
		HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_SET);
	} else if (channel == SCOPE_CH1) {
		HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(spi, (uint8_t*)&cmd, 1, 100);
		HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_SET);
	}

	return;
}

void DRV_SPI_SetPgaGain(SPI_HandleTypeDef* spi, scope_channel_t channel, pga_gain_t gain) {
	uint16_t cmd = 0x0000;

	cmd = (0b111 & gain);
	// command = write to register
	cmd |= (0b010 << 13);
	// set gain register
	// set gain

	//cmd = ((cmd & 0xFF) << 8) | ((cmd & 0xFF00) >> 8);

	if (channel == SCOPE_CH0) {
		HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(spi, (uint8_t*)&cmd, 1, 100);
		HAL_GPIO_WritePin(CSn_CH0_GPIO_Port, CSn_CH0_Pin, GPIO_PIN_SET);
	} else if (channel == SCOPE_CH1) {
		HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(spi, (uint8_t*)&cmd, 1, 100);
		HAL_GPIO_WritePin(CSn_CH1_GPIO_Port, CSn_CH1_Pin, GPIO_PIN_SET);
	}

	return;
}
