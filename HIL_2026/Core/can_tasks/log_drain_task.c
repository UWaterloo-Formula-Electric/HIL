#include "can_types.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

static char lineBuf[52];

void logDrainTask(void const *argument)
{
    for (;;)
    {
        if (logRingTail == logRingHead) { osDelay(1); continue; }

        HIL_RxLogEntry_t *e = &logRing[logRingTail];

        if (e->id == 0xFFFFFFFF)
        {
            uint32_t err = ((uint32_t)e->data[0] << 24)
                         | ((uint32_t)e->data[1] << 16)
                         | ((uint32_t)e->data[2] <<  8)
                         |  (uint32_t)e->data[3];
            snprintf(lineBuf, sizeof(lineBuf),
                     "%lu.%03lu CAN_ERR 0x%08lX\r\n",
                     e->timestamp_ms / 1000,
                     e->timestamp_ms % 1000, err);
        }
        else
        {
            int pos = snprintf(lineBuf, sizeof(lineBuf),
                               "%lu.%03lu can0 %08lX#",
                               e->timestamp_ms / 1000,
                               e->timestamp_ms % 1000, e->id);
            for (uint8_t i = 0; i < e->dlc && pos < 48; i++)
                pos += snprintf(lineBuf + pos, sizeof(lineBuf) - pos,
                                "%02X", e->data[i]);
            lineBuf[pos++] = '\r'; lineBuf[pos++] = '\n';
            lineBuf[pos]   = '\0';
        }

        HAL_UART_Transmit(&huart4, (uint8_t *)lineBuf, strlen(lineBuf), 10);
        logRingTail = (logRingTail + 1) % LOG_RING_SIZE;
    }
}
