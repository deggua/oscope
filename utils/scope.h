#ifndef SCOPE_H
#define SCOPE_H

typedef enum { SCOPE_CH0 = 0, SCOPE_CH1 = 1 } scope_channel_t;

typedef enum { ADC_RUNNING, ADC_TRIGGERED, ADC_AVAILABLE } adc_state_t;

typedef enum { EDGE_RISING, EDGE_FALLING, EDGE_BOTH } edge_t;

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
    AMPLIFICATION_384X
} amplification_t;

typedef enum { ATTENUATION_X1, ATTENUATION_X10 } attenuation_t;

typedef struct {
    float  threshold;
    edge_t edge;
} trigger_t;

typedef struct {
    trigger_t       trigger;
    amplification_t amplification;
    attenuation_t   attenuation;
} frontend_state_t;

typedef struct {
    float offset[16];
    float scalar[16];
} calibration_t;

typedef struct {
    calibration_t CH0, CH1;
} scope_calibration_t;

#endif