#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>

#include "utils/scope.h"

typedef enum {
    AMPLIFICATION_1X = 0,
    AMPLIFICATION_2X,
    AMPLIFICATION_4X,
    AMPLIFICATION_5X,
    AMPLIFICATION_8X,
    AMPLIFICATION_10X,
    AMPLIFICATION_12X,
    AMPLIFICATION_16X,
    AMPLIFICATION_24X,
    AMPLIFICATION_32X,
    AMPLIFICATION_48X,
    AMPLIFICATION_60X,
    AMPLIFICATION_96X,
    AMPLIFICATION_120X,
    AMPLIFICATION_192X,
    AMPLIFICATION_384X,
    AMPLIFICATION_LAST
} amplification_t;

typedef enum { ATTENUATION_X1 = 0, ATTENUATION_X10 } attenuation_t;

typedef struct {
    amplification_t amplification;
    attenuation_t   attenuation;
    float           offset;
} frontend_state_t;

typedef struct {
    float offset[16];
    float scalar[16];
} calibration_t;


extern float attenuation[2];
extern float amplification[16];

void PROC_SetGain(scope_channel_t channel, amplification_t gain);

void PROC_SetAttenuation(scope_channel_t channel, attenuation_t atten);

void PROC_SetOffset(scope_channel_t channel, float volt_offset);

void PROC_SetSpan(scope_channel_t channel, float volt_span);

float PROC_SampleVoltage(scope_channel_t channel);

float PROC_SampleVoltageCalibrated(scope_channel_t channel);

#endif
