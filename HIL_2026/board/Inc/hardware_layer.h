#ifndef HARDWARE_LAYER_H
#define HARDWARE_LAYER_H

#include "pin_init.h"
#include "stm32f7xx_hal.h"

void Hardware_Init(void);
void read_GPIO_();
void write_GPIO_();
void write_DAC_(void);
void read_PWM_(void);
void write_PWM_(void);
void read_ADC_(void);
#endif