#ifndef HIL_TYPES_H
#define HIL_TYPES_H

#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include <stdint.h>
#include <stdbool.h>

// Mode enum
typedef enum {
    MODE_CLI,   // canSimTask sends DBC-encoded frames
    MODE_LOG,   // canLogTask drives the bus from log file
} HIL_Mode_t;

// Log playback state
typedef enum {
    LOG_STOPPED,
    LOG_PLAYING,
    LOG_PAUSED,
} HIL_LogState_t;

// CAN command (CLI → canSimTask)
typedef struct {
    uint32_t extId;
    uint8_t  data[8];
    uint8_t  length;
} HIL_CanCmd_t;

// Output command (CLI → outputTask)
typedef enum {
    OUTPUT_GPIO_SET,
    OUTPUT_GPIO_TOGGLE,
    OUTPUT_DAC_SET,
    OUTPUT_PWM_SET,
} HIL_OutputType_t;

typedef struct {
    HIL_OutputType_t type;
    uint32_t         pin;
    GPIO_TypeDef    *port;
    uint32_t         value;
} HIL_OutputCmd_t;

// RX log ring buffer entry
#define LOG_RING_SIZE 512
typedef struct {
    uint32_t timestamp_ms;
    uint32_t id;
    uint8_t  data[8];
    uint8_t  dlc;
} HIL_RxLogEntry_t;

// Shared vehicle state
typedef struct {
    HIL_Mode_t     mode;
    HIL_LogState_t logState;

    // BMU / Battery
    uint32_t packVoltageMV;
    int16_t  packCurrentMA;
    uint8_t  prechargeState;

    // Motor Controller
    uint8_t  invVsmState;
    uint8_t  invState;
    uint8_t  invEnableLockout;
    uint8_t  invEnableState;

    // WSB Wheel Speeds (raw = kph / 0.04)
    int16_t  rrSpeedKPH_x100;
    int16_t  rlSpeedKPH_x100;
    int16_t  flSpeedKPH_x100;
    int16_t  frSpeedKPH_x100;

    // PDU
    uint8_t  pduCarStateLV;
    uint8_t  pduCarStateHV;
    uint8_t  pduCarStateEM;

    osMutexId mutex;
} HIL_VehicleState_t;

extern HIL_VehicleState_t gHILState;
extern osMessageQId       xCanTxQueue;
extern osMessageQId       xOutputQueue;
extern HIL_RxLogEntry_t   logRing[];
extern volatile uint32_t  logRingHead;
extern volatile uint32_t  logRingTail;

#endif /* HIL_TYPES_H */
