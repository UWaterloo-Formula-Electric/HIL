#include "can_types.h"
#include "can_signals.h"  // generated encode functions
#include "can.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

// Message IDs from 2024CAR.dbc (decimal → hex)
#define ID_BMU_STATE_BATTERY_HV  0x87E1C01  // 2281701889
#define ID_PRECHARGE_STATE       0x88CE001  // 2295660289
#define ID_MC_INTERNAL_STATES    0x8CF0401  // 2365565441
#define ID_WSBRR_SPEED           0x83E100B  // 2214593035
#define ID_WSBRL_SPEED           0x83E100A  // 2214593034
#define ID_PDU_CAR_STATE         0x8BD0003  // 2343239427

HIL_VehicleState_t gHILState = {0};

void canSimTask(void const *argument)
{
    gHILState.mutex = osMutexCreate(NULL);

    // Default state: LV up, HV off, MC not ready, lockout active
    gHILState.mode             = MODE_CLI;
    gHILState.logState         = LOG_STOPPED;
    gHILState.pduCarStateLV    = 1;
    gHILState.invEnableLockout = 1;

    uint32_t tick = osKernelSysTick();

    for (;;)
    {
        osMutexWait(gHILState.mutex, portMAX_DELAY);
        HIL_Mode_t mode = gHILState.mode;
        osMutexRelease(gHILState.mutex);

        if (mode == MODE_CLI)
        {
            // Drain on-demand frames from CLI
            HIL_CanCmd_t cmd;
            while (osMessageGet(xCanTxQueue, /*&cmd,*/ 0) == osOK)
                HIL_CAN_Transmit(&hcan3, cmd.extId, cmd.length, cmd.data);

            // 10 ms periodic sends
            send_MC_Internal_States();
            send_WSB_Speeds();

            // 100 ms periodic sends (every 10th tick)
            static uint8_t slow = 0;
            if (++slow >= 10) {
                slow = 0;
                send_BMU_stateBatteryHV();
                send_PrechargeState();
                send_PDU_Car_State();
            }
        }
        // MODE_LOG: yield, canLogTask drives the bus

        osDelayUntil(&tick, 10);
    }
}
