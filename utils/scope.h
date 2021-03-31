#ifndef SCOPE_H
#define SCOPE_H

#define SCOPE_VREF 3.28f

typedef enum { SCOPE_CH0 = 0, SCOPE_CH1 = 1 } scope_channel_t;

typedef enum { ADC_RUNNING, ADC_TRIGGERED, ADC_AVAILABLE } adc_state_t;

typedef enum { EDGE_RISING, EDGE_FALLING, EDGE_BOTH } edge_t;

typedef struct {
    float  threshold;
    edge_t edge;
} trigger_t;

#endif