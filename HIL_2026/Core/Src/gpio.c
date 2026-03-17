/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
  bool is_active_low_button;
} HIL_VCU_GpioDescriptor;

static bool HIL_VCU_GetDescriptor(HIL_VCU_GpioSignal signal,
                                  HIL_VCU_GpioDescriptor *descriptor)
{
  if (descriptor == 0)
  {
    return false;
  }

  switch (signal)
  {
    case HIL_VCU_SIGNAL_BTN_HV:
      descriptor->port = HIL_VCU_BTN_HV_GPIO_Port;
      descriptor->pin = HIL_VCU_BTN_HV_Pin;
      descriptor->is_active_low_button = true;
      return true;
    case HIL_VCU_SIGNAL_BTN_EM:
      descriptor->port = HIL_VCU_BTN_EM_GPIO_Port;
      descriptor->pin = HIL_VCU_BTN_EM_Pin;
      descriptor->is_active_low_button = true;
      return true;
    case HIL_VCU_SIGNAL_BTN_TC:
      descriptor->port = HIL_VCU_BTN_TC_GPIO_Port;
      descriptor->pin = HIL_VCU_BTN_TC_Pin;
      descriptor->is_active_low_button = true;
      return true;
    case HIL_VCU_SIGNAL_BTN_ENDUR:
      descriptor->port = HIL_VCU_BTN_ENDUR_GPIO_Port;
      descriptor->pin = HIL_VCU_BTN_ENDUR_Pin;
      descriptor->is_active_low_button = true;
      return true;
    case HIL_VCU_SIGNAL_BUT1:
      descriptor->port = HIL_VCU_BUT1_GPIO_Port;
      descriptor->pin = HIL_VCU_BUT1_Pin;
      descriptor->is_active_low_button = false;
      return true;
    case HIL_VCU_SIGNAL_BUT2:
      descriptor->port = HIL_VCU_BUT2_GPIO_Port;
      descriptor->pin = HIL_VCU_BUT2_Pin;
      descriptor->is_active_low_button = false;
      return true;
    case HIL_VCU_SIGNAL_BUT3:
      descriptor->port = HIL_VCU_BUT3_GPIO_Port;
      descriptor->pin = HIL_VCU_BUT3_Pin;
      descriptor->is_active_low_button = false;
      return true;
    default:
      return false;
  }
}

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, LDAC_1_Pin|GPIO12V_6_Pin|GPIO12V_5_Pin|GPIO12V_4_Pin
                          |GPIO12V_3_Pin|GPIO12V_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LDAC_2_GPIO_Port, LDAC_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LDAC_3_GPIO_Port, LDAC_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, GPIO5V_2_Pin|GPIO5V_1_Pin|GPIO5V_9_Pin|GPIO5V_8J3_Pin
                          |GPIO5V_7J4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO12V_1_GPIO_Port, GPIO12V_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO3V_1_Pin|GPIO3V_2_Pin|GPIO3V_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO3V_5_Pin|GPIO3V_6_Pin|GPIO3V_7_Pin|GPIO3V_8_Pin
                          |GPIO3V_9_Pin|GPIO3V_10_Pin|GPIO3V_11_Pin|GPIO3V_12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LDAC_4_GPIO_Port, LDAC_4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LDAC_1_Pin GPIO12V_6_Pin GPIO12V_5_Pin GPIO12V_4_Pin
                           GPIO12V_3_Pin GPIO12V_2_Pin */
  GPIO_InitStruct.Pin = LDAC_1_Pin|GPIO12V_6_Pin|GPIO12V_5_Pin|GPIO12V_4_Pin
                          |GPIO12V_3_Pin|GPIO12V_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : LDAC_2_Pin */
  GPIO_InitStruct.Pin = LDAC_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LDAC_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LDAC_3_Pin */
  GPIO_InitStruct.Pin = LDAC_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LDAC_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO5V_8_Pin GPIO5V_7_Pin GPIO3V_13_Pin */
  GPIO_InitStruct.Pin = GPIO5V_8_Pin|GPIO5V_7_Pin|GPIO3V_13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO5V_6_Pin GPIO5V_5_Pin GPIO5V_4_Pin */
  GPIO_InitStruct.Pin = GPIO5V_6_Pin|GPIO5V_5_Pin|GPIO5V_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO5V_3_Pin */
  GPIO_InitStruct.Pin = GPIO5V_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO5V_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO5V_2_Pin GPIO5V_1_Pin GPIO5V_9_Pin GPIO5V_8J3_Pin
                           GPIO5V_7J4_Pin */
  GPIO_InitStruct.Pin = GPIO5V_2_Pin|GPIO5V_1_Pin|GPIO5V_9_Pin|GPIO5V_8J3_Pin
                          |GPIO5V_7J4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO12V_1_Pin */
  GPIO_InitStruct.Pin = GPIO12V_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO12V_1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO12V_10_Pin */
  GPIO_InitStruct.Pin = GPIO12V_10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO12V_10_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO12V_11_Pin GPIO12V_12_Pin GPIO12V_13_Pin GPIO12V_14_Pin
                           GPIO12V_15_Pin GPIO12V_16_Pin GPIO12V_17_Pin */
  GPIO_InitStruct.Pin = GPIO12V_11_Pin|GPIO12V_12_Pin|GPIO12V_13_Pin|GPIO12V_14_Pin
                          |GPIO12V_15_Pin|GPIO12V_16_Pin|GPIO12V_17_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO12V_18_Pin GPIO12V_19_Pin GPIO12V_20_Pin GPIO12V_21_Pin */
  GPIO_InitStruct.Pin = GPIO12V_18_Pin|GPIO12V_19_Pin|GPIO12V_20_Pin|GPIO12V_21_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO3V_1_Pin GPIO3V_2_Pin GPIO3V_4_Pin */
  GPIO_InitStruct.Pin = GPIO3V_1_Pin|GPIO3V_2_Pin|GPIO3V_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO3V_3_Pin */
  GPIO_InitStruct.Pin = GPIO3V_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIO3V_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO3V_5_Pin GPIO3V_6_Pin GPIO3V_7_Pin GPIO3V_8_Pin
                           GPIO3V_9_Pin GPIO3V_10_Pin GPIO3V_11_Pin GPIO3V_12_Pin */
  GPIO_InitStruct.Pin = GPIO3V_5_Pin|GPIO3V_6_Pin|GPIO3V_7_Pin|GPIO3V_8_Pin
                          |GPIO3V_9_Pin|GPIO3V_10_Pin|GPIO3V_11_Pin|GPIO3V_12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO5V_14_Pin GPIO5V_15_Pin */
  GPIO_InitStruct.Pin = GPIO5V_14_Pin|GPIO5V_15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LDAC_4_Pin */
  GPIO_InitStruct.Pin = LDAC_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LDAC_4_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void HIL_VCU_GPIO_Init(void)
{
  /* default to high - buttons*/
  (void)HIL_VCU_ReleaseAllButtons();
}

bool HIL_VCU_WriteRaw(HIL_VCU_GpioSignal signal, GPIO_PinState state)
{
  HIL_VCU_GpioDescriptor descriptor;

  if (!HIL_VCU_GetDescriptor(signal, &descriptor))
  {
    return false;
  }

  HAL_GPIO_WritePin(descriptor.port, descriptor.pin, state);
  return true;
}

bool HIL_VCU_ReadRaw(HIL_VCU_GpioSignal signal, GPIO_PinState *state)
{
  HIL_VCU_GpioDescriptor descriptor;

  if ((state == 0) || !HIL_VCU_GetDescriptor(signal, &descriptor))
  {
    return false;
  }

  *state = HAL_GPIO_ReadPin(descriptor.port, descriptor.pin);
  return true;
}

bool HIL_VCU_SetButtonPressed(HIL_VCU_GpioSignal signal, bool pressed)
{
  HIL_VCU_GpioDescriptor descriptor;
  GPIO_PinState state;

  if (!HIL_VCU_GetDescriptor(signal, &descriptor) ||
      !descriptor.is_active_low_button)
  {
    return false;
  }

  state = pressed ? GPIO_PIN_RESET : GPIO_PIN_SET;
  return HIL_VCU_WriteRaw(signal, state);
}

bool HIL_VCU_ReleaseAllButtons(void)
{
  bool ok = true;

  ok = HIL_VCU_SetButtonPressed(HIL_VCU_SIGNAL_BTN_HV, false) && ok;
  ok = HIL_VCU_SetButtonPressed(HIL_VCU_SIGNAL_BTN_EM, false) && ok;
  ok = HIL_VCU_SetButtonPressed(HIL_VCU_SIGNAL_BTN_TC, false) && ok;
  ok = HIL_VCU_SetButtonPressed(HIL_VCU_SIGNAL_BTN_ENDUR, false) && ok;

  return ok;
}

/* USER CODE END 2 */
