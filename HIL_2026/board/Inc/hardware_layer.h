#ifndef HARDWARE_LAYER_H
#define HARDWARE_LAYER_H

#include "pin_init.h"
#include "stm32f7xx_hal.h"

void Hardware_Init(void);
void read_GPIO_3V3();
void read_GPIO_5V();
void write_GPIO_3V3();
void write_GPIO_5V();
void read_analog_3V3(void);
void read_analog_5V(void);
void write_analog_3V3(void);
void write_analog_5V(void);

#endif