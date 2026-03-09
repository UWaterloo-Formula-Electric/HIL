#ifndef SIGNAL_MAP_H
#define SIGNAL_MAP_H
#include "stm32f7xx_hal.h"
#include "hardware_layer.h"

typedef enum Signals {
    VCU_SIGNAL_1,
    VCU_SIGNAL_2,
    VCU_SIGNAL_3,
    VCU_SIGNAL_4,
    VCU_SIGNAL_5,
    VCU_SIGNAL_6,
    VCU_SIGNAL_7,
    VCU_SIGNAL_8,
    BMU_SIGNAL_1,
    BMU_SIGNAL_2,
    BMU_SIGNAL_3,
    BMU_SIGNAL_4,
    BMU_SIGNAL_5,
    BMU_SIGNAL_6,
    BMU_SIGNAL_7,
    BMU_SIGNAL_8,
    PDU_SIGNAL_1,
    PDU_SIGNAL_2,
    PDU_SIGNAL_3,
    PDU_SIGNAL_4,
    PDU_SIGNAL_5,
    PDU_SIGNAL_6,
    PDU_SIGNAL_7,
    PDU_SIGNAL_8,
} Signals;

typedef enum logic_level{
    LOW,
    HIGH_3_3V,
    HIGH_5V,
    HIGH_12V,
} logic_level;

typedef struct GPIO_Signal_Map {
    Signals signal;
    GPIO_TypeDef *gpio;
    uint16_t pin;
    logic_level level;
} GPIO_Signal_Map;

typedef struct DAC_Signal_Map {
    Signals signal;
    DAC_HandleTypeDef *dac;
    uint32_t dac_channel;
} DAC_Signal_Map;

typedef struct PWM_Signal_Map {
    Signals signal;
    TIM_HandleTypeDef *tim;
    uint32_t pwm_channel;
} PWM_Signal_Map;

typedef struct ADC_Signal_Map {
    Signals signal;
    ADC_HandleTypeDef *adc;
    uint32_t adc_channel;
} ADC_Signal_Map;

