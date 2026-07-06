#include "control_state_machine.h"

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "stdbool.h"

#include "bsp.h"
#include "watchdog.h"

#define MAIN_TASK_ID 1
#define MAIN_TASK_PERIOD_MS 1000

static uint32_t selectCANMode(uint32_t event);
static uint32_t selectGPIOMode(uint32_t event);
static uint32_t selectDACMode(uint32_t event);
static uint32_t exitMode(uint32_t event);

static bool idle_mode = false;
static bool can_mode = false;
static bool gpio_mode = false;
static bool dac_mode = false;

static Transition_t hilTransitionTable[] = {
    {STATE_Idle,    CLI_CAN_Select,     &selectCANMode},
    {STATE_Idle,    CLI_GPIO_Select,    &selectGPIOMode},
    {STATE_Idle,    CLI_DAC_Select,     &selectDACMode},
    {STATE_CAN,     CLI_Exit_Mode,      &exitMode},
    {STATE_GPIO,    CLI_Exit_Mode,      &exitMode},
    {STATE_DAC,     CLI_Exit_Mode,      &exitMode},
};

static uint32_t selectCANMode(uint32_t event) 
{
    const HIL_States_t current_state = fsmGetState(&HILFsmHandle);
    HIL_States_t new_state = STATE_Idle;

    switch (current_state) {
        case STATE_Idle:
            can_mode = true;
            new_state = STATE_CAN;
            break;
        case STATE_CAN:
            new_state = STATE_CAN;
            break;
        case STATE_GPIO:
        case STATE_DAC:
        default:
            DEBUG_PRINT("Fault: Cannot transition from %lu to CAN mode\n", current_state);
            new_state = STATE_Idle;
            break;
    }

    return new_state;
}

static uint32_t selectGPIOMode(uint32_t event)
{
    const HIL_States_t current_state = fsmGetState(&HILFsmHandle);
    HIL_States_t new_state = STATE_Idle;

    switch (current_state) {
        case STATE_Idle:
            gpio_mode = true;
            new_state = STATE_GPIO;
            break;
        case STATE_CAN:
        case STATE_GPIO:
            new_state = STATE_GPIO;
            break;
        case STATE_DAC:
        default:
            DEBUG_PRINT("Fault: Cannot transition from %lu to CAN mode\n", current_state);
            new_state = STATE_Idle;
            break;
    }

    return new_state;
}

static uint32_t selectDACMode(uint32_t event)
{
   const HIL_States_t current_state = fsmGetState(&HILFsmHandle);
    HIL_States_t new_state = STATE_Idle;

    switch (current_state) {
        case STATE_Idle:
            gpio_mode = true;
            new_state = STATE_DAC;
            break;
        case STATE_CAN:
        case STATE_GPIO:
        case STATE_DAC:
            new_state = STATE_DAC;
            break;
        default:
            DEBUG_PRINT("Fault: Cannot transition from %lu to CAN mode\n", current_state);
            new_state = STATE_Idle;
            break;
    }

    return new_state;
}

static uint32_t exitMode(uint32_t event)
{
       const HIL_States_t current_state = fsmGetState(&HILFsmHandle);
    HIL_States_t new_state = STATE_Idle;

    switch (current_state) {
        case STATE_CAN:
        case STATE_GPIO:
        case STATE_DAC:
        case STATE_Idle:
            idle_mode = true;
            new_state = STATE_Idle;
            break;
        default:
            DEBUG_PRINT("Fault: Cannot transition from %lu to CAN mode\n", current_state);
            new_state = STATE_Idle;
            break;
    }

    return new_state;
}
