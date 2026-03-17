/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
typedef enum
{
  HIL_VCU_SIGNAL_BTN_HV = 0,
  HIL_VCU_SIGNAL_BTN_EM,
  HIL_VCU_SIGNAL_BTN_TC,
  HIL_VCU_SIGNAL_BTN_ENDUR,
  HIL_VCU_SIGNAL_BUT1,
  HIL_VCU_SIGNAL_BUT2,
  HIL_VCU_SIGNAL_BUT3
} HIL_VCU_GpioSignal;

/*
 * The four BTN_* signals r handled as active-low input. these 3.3v are being used, not sure if right?
 */
#define HIL_VCU_BTN_HV_Pin GPIO3V_1_Pin
#define HIL_VCU_BTN_HV_GPIO_Port GPIO3V_1_GPIO_Port
#define HIL_VCU_BTN_EM_Pin GPIO3V_2_Pin
#define HIL_VCU_BTN_EM_GPIO_Port GPIO3V_2_GPIO_Port
#define HIL_VCU_BTN_TC_Pin GPIO3V_4_Pin
#define HIL_VCU_BTN_TC_GPIO_Port GPIO3V_4_GPIO_Port
#define HIL_VCU_BTN_ENDUR_Pin GPIO3V_5_Pin
#define HIL_VCU_BTN_ENDUR_GPIO_Port GPIO3V_5_GPIO_Port

#define HIL_VCU_BUT1_Pin GPIO3V_6_Pin
#define HIL_VCU_BUT1_GPIO_Port GPIO3V_6_GPIO_Port
#define HIL_VCU_BUT2_Pin GPIO3V_7_Pin
#define HIL_VCU_BUT2_GPIO_Port GPIO3V_7_GPIO_Port
#define HIL_VCU_BUT3_Pin GPIO3V_8_Pin
#define HIL_VCU_BUT3_GPIO_Port GPIO3V_8_GPIO_Port

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void HIL_VCU_GPIO_Init(void);
bool HIL_VCU_WriteRaw(HIL_VCU_GpioSignal signal, GPIO_PinState state);
bool HIL_VCU_ReadRaw(HIL_VCU_GpioSignal signal, GPIO_PinState *state);
bool HIL_VCU_SetButtonPressed(HIL_VCU_GpioSignal signal, bool pressed);
bool HIL_VCU_ReleaseAllButtons(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

