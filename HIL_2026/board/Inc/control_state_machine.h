#ifndef HIL_STATE_MACHINE
#define HIL_STATE_MACHINE
#include "state_machine.h"

typedef enum HIL_States_t {
    STATE_Idle = 0U,
    STATE_CAN,
    STATE_GPIO,
    STATE_DAC,
    STATE_ANY,
} HIL_States_t;

typedef enum HIL_Events_t {
    CLI_CAN_Select = 0,
    CLI_GPIO_Select,
    CLI_DAC_Select,
    CLI_Exit_Mode,
} HIL_Events_t;

HAL_StatusTypeDef hilFsmInit();


extern FSM_Handle_Struct HILFsmHandle;
#endif
