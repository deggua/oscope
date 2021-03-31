#include "processing/control.h"

#include <stdbool.h>

#include "config.h"
#include "drivers/adc.h"
#include "drivers/spi.h"
#include "utils/scope.h"

float attenuation[2] = {1.0f, 10.0f};
float amplification[16] =
    {1.0f, 2.0f, 4.0f, 5.0f, 8.0f, 10.0f, 12.0f, 16.0f, 24.0f, 32.0f, 48.0f, 60.0f, 96.0f, 120.0f, 192.0f, 384.0f};

calibration_t cal_vals = {
		.offset = {2.89411783f, 1.3184315f, 0.530588329f, 0.373019576f, 0.136666596f, 0.0450195372f, 0.00535935163f, -0.0522548705f, -0.179542482f, -0.259264708f, -0.314869285f, -0.251895428f, -0.154754907f, -0.122732013f, -0.0740277842f, -0.0339991823f},
		.scalar = {0.903998733f, 0.903998852f, 0.905754149f, 0.899516225f, 0.905754268f, 0.902384579f, 0.901378512f, 0.888501704f, 0.888501704f, 1.15176165f, 1.08165443f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }
};

frontend_state_t CH0 = {0}, CH1 = {0};

void PROC_SetGain(scope_channel_t channel, amplification_t gain) {
    if (channel == SCOPE_CH0) {
        CH0.amplification = gain;
    } else if (channel == SCOPE_CH1) {
        CH1.amplification = gain;
    }

    switch (gain) {
        case AMPLIFICATION_1X:
        case AMPLIFICATION_2X:
        case AMPLIFICATION_4X:
        case AMPLIFICATION_5X:
        case AMPLIFICATION_8X:
        case AMPLIFICATION_10X:
        case AMPLIFICATION_16X:
        case AMPLIFICATION_32X: {
            DRV_SPI_SetPgaChannel(channel, PGA_CH_1X);
        } break;

        case AMPLIFICATION_12X:
        case AMPLIFICATION_24X:
        case AMPLIFICATION_48X:
        case AMPLIFICATION_60X:
        case AMPLIFICATION_96X:
        case AMPLIFICATION_120X:
        case AMPLIFICATION_192X:
        case AMPLIFICATION_384X: {
            DRV_SPI_SetPgaChannel(channel, PGA_CH_12X);
        } break;

        default: break;
    }

    switch (gain) {
        case AMPLIFICATION_1X:
        case AMPLIFICATION_12X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_1X);
        } break;

        case AMPLIFICATION_2X:
        case AMPLIFICATION_24X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_2X);
        } break;

        case AMPLIFICATION_4X:
        case AMPLIFICATION_48X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_4X);
        } break;

        case AMPLIFICATION_5X:
        case AMPLIFICATION_60X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_5X);
        } break;

        case AMPLIFICATION_8X:
        case AMPLIFICATION_96X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_8X);
        } break;

        case AMPLIFICATION_10X:
        case AMPLIFICATION_120X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_10X);
        } break;

        case AMPLIFICATION_16X:
        case AMPLIFICATION_192X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_16X);
        } break;

        case AMPLIFICATION_32X:
        case AMPLIFICATION_384X: {
            DRV_SPI_SetPgaGain(channel, PGA_GAIN_32X);
        } break;

        default: break;
    }

    return;
}

void PROC_SetAttenuation(scope_channel_t channel, attenuation_t atten) {
    if (channel == SCOPE_CH0) {
        CH0.attenuation = atten;
    } else if (channel == SCOPE_CH1) {
        CH1.attenuation = atten;
    }

    return;
}

static amplification_t PROC_CalcIdealGain(attenuation_t probe, float volt_span) {
    float defaultSpan = SCOPE_VREF;
    float desiredSpan = volt_span;

    defaultSpan = defaultSpan * attenuation[probe];
    for (int ii = 0; ii < lengthof(amplification); ii++) {
        if (defaultSpan / amplification[ii] <= desiredSpan) {
            return (amplification_t)ii;
        }
    }

    return AMPLIFICATION_384X;
}

static float PROC_CalcCenterOffset(amplification_t gain) {
    float desiredCenter = SCOPE_VREF / 2;
    float idealOffset   = desiredCenter / amplification[gain];
    return idealOffset;
}

void PROC_SetOffset(scope_channel_t channel, float volt_offset) {
    float offsetUser;
    if (channel == SCOPE_CH0) {
        float offsetCenter = PROC_CalcCenterOffset(CH0.amplification);
        offsetUser         = offsetCenter + volt_offset;
        CH0.offset         = offsetUser;
    } else if (channel == SCOPE_CH1) {
        float offsetCenter = PROC_CalcCenterOffset(CH1.amplification);
        offsetUser         = offsetCenter + volt_offset;
        CH1.offset         = offsetUser;
    }

    DRV_SPI_SetDacVoltage(channel, offsetUser);
    return;
}

void PROC_SetSpan(scope_channel_t channel, float volt_span) {
    if (channel == SCOPE_CH0) {
        PROC_SetGain(channel, PROC_CalcIdealGain(CH0.attenuation, volt_span));
    } else if (channel == SCOPE_CH1) {
        PROC_SetGain(channel, PROC_CalcIdealGain(CH1.attenuation, volt_span));
    }

    return;
}

float PROC_SampleVoltage(scope_channel_t channel) {
    float voltADC = DRV_ADC_ReadVoltage(channel);
    float voltOffset, voltAmplification, voltAttenuation;
    amplification_t ampSel;
	attenuation_t attenSel;
    if (channel == SCOPE_CH0) {
        voltOffset        = CH0.offset;
        voltAmplification = amplification[CH0.amplification];
        ampSel = CH0.amplification;
        attenSel = CH0.attenuation;
        voltAttenuation   = attenuation[CH0.attenuation];
    } else if (channel == SCOPE_CH1) {
        voltOffset        = CH1.offset;
        voltAmplification = amplification[CH1.amplification];
        ampSel = CH0.amplification;
        attenSel = CH0.attenuation;
        voltAttenuation   = attenuation[CH1.attenuation];
    }

    float voltReal = -1.0f * voltAttenuation * (voltADC / voltAmplification - voltOffset);
    return voltReal;
}

float PROC_SampleVoltageCalibrated(scope_channel_t channel) {
    amplification_t ampSel;
	attenuation_t attenSel;

	if (channel == SCOPE_CH0) {
		ampSel = CH0.amplification;
		attenSel = CH0.attenuation;
	} else if (channel == SCOPE_CH1) {
		ampSel = CH1.amplification;
		attenSel = CH1.attenuation;
	}

	float voltMeasured = PROC_SampleVoltage(channel);

	float attenFactor = 1.0f;
	if (attenSel == ATTENUATION_X1) {
		attenFactor = 10.0f;
	}
    float voltCal = cal_vals.scalar[ampSel] * (voltMeasured + attenFactor * cal_vals.offset[ampSel]);
    return voltCal;
}
