#ifndef __BSP_H
#define __BSP_H

// Adjusted for HIL

#include "adc.h"
#include "can.h"
#include "iwdg.h"
#include "main.h"
#include "stdbool.h"
#include "tim.h"
#include "usart.h"

#include "stm32f7xx_hal.h"

// TODO: Set these to the right values (double check all these values)
#define DEBUG_UART_HANDLE huart4
#define CAN_HANDLE hcan3
#define IWDG_HANDLE hiwdg

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#pragma message "BOARD_TYPE_NUCLEO_F7: " #BOARD_TYPE_NUCLEO_F7

#endif

// Comment out to remove debug printing
#define DEBUG_ON

// Comment out to remove error printing
#define ERROR_PRINT_ON

#define CONSOLE_PRINT_ON
