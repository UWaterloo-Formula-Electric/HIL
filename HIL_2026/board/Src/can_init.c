#include "can.h"
#include "can_types.h"

void userInit(void)
{
    // Accept-all filter for CAN3
    CAN_FilterTypeDef filter = {0};
    filter.FilterBank           = 0;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation     = ENABLE;
    // All ID/mask fields 0 from {0} init — accept every frame
    if (HAL_CAN_ConfigFilter(&hcan3, &filter) != HAL_OK)
        Error_Handler();

    if (HAL_CAN_Start(&hcan3) != HAL_OK)
        Error_Handler();

    if (HAL_CAN_ActivateNotification(&hcan3, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
        Error_Handler();

    if (HAL_CAN_ActivateNotification(&hcan3, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
        Error_Handler();
}
