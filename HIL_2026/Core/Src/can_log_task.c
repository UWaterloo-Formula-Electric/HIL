#include "can_types.h"
#include "can.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define LOG_LINE_MAX 80

static uint8_t uartByte;
static char    lineBuf[LOG_LINE_MAX];
static uint8_t lineLen = 0;

static void playLine(const char *line)
{
    const char *hash = strchr(line, '#');
    if (!hash) return;

    const char *idStart = hash - 1;
    while (idStart > line && *idStart != ' ') idStart--;
    idStart++;

    uint32_t extId = (uint32_t)strtoul(idStart, NULL, 16);

    const char *p = hash + 1;
    uint8_t data[8] = {0};
    uint8_t len = 0;
    while (len < 8 && *p && *p != '\n' && *p != '\r') {
        char b[3] = {p[0], p[1] ? p[1] : '0', '\0'};
        data[len++] = (uint8_t)strtoul(b, NULL, 16);
        p += 2;
    }
    HIL_CAN_Transmit(&hcan3, extId, len, data);
}

void canLogTask(void const *argument)
{
    for (;;)
    {
        // Outer wait: block until MODE_LOG + LOG_PLAYING
        for (;;) {
            osMutexWait(gHILState.mutex, portMAX_DELAY);
            bool go = (gHILState.mode     == MODE_LOG) &&
                      (gHILState.logState == LOG_PLAYING);
            osMutexRelease(gHILState.mutex);
            if (go) break;
            osDelay(20);
        }

        // Playback loop
        while (1)
        {
            osMutexWait(gHILState.mutex, portMAX_DELAY);
            HIL_LogState_t st = gHILState.logState;
            HIL_Mode_t     md = gHILState.mode;
            osMutexRelease(gHILState.mutex);

            if (md == MODE_CLI || st == LOG_STOPPED) break;

            if (st == LOG_PAUSED) { osDelay(20); continue; }

            // Receive next line from UART
            if (HAL_UART_Receive(&huart4, &uartByte, 1, 10) == HAL_OK) {
                if (uartByte == '\n' || uartByte == '\r') {
                    if (lineLen > 0) {
                        lineBuf[lineLen] = '\0';
                        if (lineBuf[0] == '!') {
                            osMutexWait(gHILState.mutex, portMAX_DELAY);
                            gHILState.logState = LOG_STOPPED;
                            gHILState.mode     = MODE_CLI;
                            osMutexRelease(gHILState.mutex);
                        } else if (strchr(lineBuf, '#')) {
                            playLine(lineBuf);
                        }
                        lineLen = 0;
                    }
                } else if (lineLen < LOG_LINE_MAX - 1) {
                    lineBuf[lineLen++] = (char)uartByte;
                }
            }
        }
    }
}
