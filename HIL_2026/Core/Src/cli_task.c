#include "main.h"      
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"   
#include "can_types.h"   
#include "can.h"         
#include "usart.h"
#include "FreeRTOS_CLI.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static BaseType_t cmd_mode(char *out, size_t len, const char *cmd)
{
    const char *arg = cmd + 5;  // skip 'mode '
    osMutexWait(gHILState.mutex, portMAX_DELAY);
    if (strncmp(arg, "cli", 3) == 0) {
        gHILState.mode = MODE_CLI;
        snprintf(out, len, "Mode: CLI\r\n");
    } else if (strncmp(arg, "log", 3) == 0) {
        gHILState.mode = MODE_LOG;
        snprintf(out, len, "Mode: LOG\r\n");
    } else {
        snprintf(out, len, "Usage: mode cli|log\r\n");
    }
    osMutexRelease(gHILState.mutex);
    return pdFALSE;
}

static BaseType_t cmd_log(char *out, size_t len, const char *cmd)
{
    const char *arg = cmd + 4;  // skip 'log '
    osMutexWait(gHILState.mutex, portMAX_DELAY);
    if (strncmp(arg, "start", 5) == 0) {
        gHILState.mode     = MODE_LOG;
        gHILState.logState = LOG_PLAYING;
        snprintf(out, len, "Log: PLAYING\r\n");
    } else if (strncmp(arg, "pause", 5) == 0) {
        gHILState.logState = LOG_PAUSED;
        snprintf(out, len, "Log: PAUSED\r\n");
    } else if (strncmp(arg, "resume", 6) == 0) {
        gHILState.logState = LOG_PLAYING;
        snprintf(out, len, "Log: PLAYING\r\n");
    } else if (strncmp(arg, "exit", 4) == 0) {
        gHILState.logState = LOG_STOPPED;
        gHILState.mode     = MODE_CLI;
        snprintf(out, len, "Log: STOPPED, mode: CLI\r\n");
    } else {
        snprintf(out, len, "Usage: log start|pause|resume|exit\r\n");
    }
    osMutexRelease(gHILState.mutex);
    return pdFALSE;
}

static BaseType_t cmd_state(char *out, size_t len, const char *cmd)
{
    const char *arg = cmd + 6;
    // Guard: only valid in CLI mode
    if (gHILState.mode != MODE_CLI) {
        snprintf(out, len, "Error: run 'log exit' first\r\n");
        return pdFALSE;
    }
    osMutexWait(gHILState.mutex, portMAX_DELAY);
    if (strncmp(arg, "lv", 2) == 0) {
        gHILState.pduCarStateLV    = 1; gHILState.pduCarStateHV = 0;
        gHILState.pduCarStateEM    = 0; gHILState.prechargeState = 0;
        gHILState.invVsmState      = 0; gHILState.invEnableLockout = 1;
        gHILState.packVoltageMV    = 0;
        snprintf(out, len, "State: LV\r\n");
    } else if (strncmp(arg, "hv", 2) == 0) {
        gHILState.pduCarStateLV    = 1; gHILState.pduCarStateHV = 1;
        gHILState.prechargeState   = 3; gHILState.invVsmState = 4;
        gHILState.invEnableLockout = 1; gHILState.packVoltageMV = 400000;
        snprintf(out, len, "State: HV\r\n");
    } else if (strncmp(arg, "em", 2) == 0) {
        gHILState.pduCarStateEM    = 1; gHILState.invVsmState = 5;
        gHILState.invEnableLockout = 0; gHILState.invEnableState = 1;
        snprintf(out, len, "State: EM\r\n");
    } else if (strncmp(arg, "running", 7) == 0) {
        gHILState.invVsmState     = 8;
        gHILState.rrSpeedKPH_x100 = (int16_t)(50.0f / 0.04f);
        gHILState.rlSpeedKPH_x100 = (int16_t)(50.0f / 0.04f);
        snprintf(out, len, "State: Running\r\n");
    } else { snprintf(out, len, "Options: lv|hv|em|running\r\n"); }
    osMutexRelease(gHILState.mutex);
    return pdFALSE;
}

static BaseType_t cmd_bmu(char *out, size_t len, const char *cmd)
{
    char sig[32]; int val;
    if (sscanf(cmd + 4, "%31s %d", sig, &val) != 2) {
        snprintf(out, len, "Usage: bmu <signal> <val>\r\n"); return pdFALSE;
    }
    osMutexWait(gHILState.mutex, portMAX_DELAY);
    if      (!strcmp(sig, "vsm"))       gHILState.invVsmState      = (uint8_t)val;
    else if (!strcmp(sig, "lockout"))   gHILState.invEnableLockout = (uint8_t)val;
    else if (!strcmp(sig, "precharge")) gHILState.prechargeState   = (uint8_t)val;
    else if (!strcmp(sig, "volts"))     gHILState.packVoltageMV    = (uint32_t)val;
    else if (!strcmp(sig, "current"))   gHILState.packCurrentMA    = (int16_t)val;
    else { osMutexRelease(gHILState.mutex);
           snprintf(out, len, "Signals: vsm|lockout|precharge|volts|current\r\n");
           return pdFALSE; }
    osMutexRelease(gHILState.mutex);
    snprintf(out, len, "bmu.%s = %d\r\n", sig, val);
    return pdFALSE;
}

static BaseType_t cmd_speed(char *out, size_t len, const char *cmd)
{
    float rr, rl;
    if (sscanf(cmd + 6, "%f %f", &rr, &rl) != 2) {
        snprintf(out, len, "Usage: speed <rr_kph> <rl_kph>\r\n"); return pdFALSE;
    }
    osMutexWait(gHILState.mutex, portMAX_DELAY);
    gHILState.rrSpeedKPH_x100 = (int16_t)(rr / 0.04f);
    gHILState.rlSpeedKPH_x100 = (int16_t)(rl / 0.04f);
    osMutexRelease(gHILState.mutex);
    snprintf(out, len, "Speed RR=%.1f RL=%.1f kph\r\n", rr, rl);
    return pdFALSE;
}

static BaseType_t cmd_can(char *out, size_t len, const char *cmd)
{
    HIL_CanCmd_t msg = {0};
    const char *p = cmd + 4;
    msg.extId = (uint32_t)strtoul(p, (char **)&p, 16);
    while (*p == ' ') p++;
    while (*p && msg.length < 8) {
        char b[3] = {p[0], p[1] ? p[1] : '0', '\0'};
        msg.data[msg.length++] = (uint8_t)strtoul(b, NULL, 16);
        p += 2; while (*p == ' ') p++;
    }
    osMessagePut(xCanTxQueue, (uint32_t)&msg, 0);
    snprintf(out, len, "Queued 0x%08lX len=%d\r\n", msg.extId, msg.length);
    return pdFALSE;
}

static const CLI_Command_Definition_t xCmds[] = {
    { cmd_mode,   "mode",   "mode cli|log\r\n",               1 },
    { cmd_log,    "log",    "log start|pause|resume|exit\r\n", 1 },
    { cmd_state,  "state",  "state lv|hv|em|running\r\n",     1 },
    { cmd_bmu,    "bmu",    "bmu <signal> <val>\r\n",          2 },
    { cmd_speed,  "speed",  "speed <rr_kph> <rl_kph>\r\n",    2 },
    { cmd_can,    "can",    "can <id_hex> <data_hex...>\r\n",  2 },
    // { cmd_gpio,   "gpio",   "gpio <port> <pin> <0|1>\r\n",     3 },
    // { cmd_dac,    "dac",    "dac <ch> <0-4095>\r\n",           2 },
    // { cmd_status, "status", "status\r\n",                       0 },
};

void hilCliTask(void const *argument)
{
    for (size_t i = 0; i < sizeof(xCmds)/sizeof(xCmds[0]); i++)
        FreeRTOS_CLIRegisterCommand(&xCmds[i]);

    static char inBuf[128];
    static char outBuf[256];
    uint8_t ch; uint8_t idx = 0;

    HAL_UART_Transmit(&huart4, (uint8_t *)"\r\nHIL> ", 7, 100);

    for (;;)
    {
        // Pause CLI input while in LOG mode
        osMutexWait(gHILState.mutex, portMAX_DELAY);
        bool inLog = (gHILState.mode == MODE_LOG);
        osMutexRelease(gHILState.mutex);
        if (inLog) { osDelay(50); continue; }

        if (HAL_UART_Receive(&huart4, &ch, 1, 100) != HAL_OK) continue;
        HAL_UART_Transmit(&huart4, &ch, 1, 10);

        if (ch == '\r' || ch == '\n') {
            if (idx == 0) { HAL_UART_Transmit(&huart4,(uint8_t*)"\r\nHIL> ",7,10); continue; }
            inBuf[idx] = '\0'; idx = 0;
            BaseType_t more;
            do {
                more = FreeRTOS_CLIProcessCommand(inBuf, outBuf, sizeof(outBuf));
                HAL_UART_Transmit(&huart4, (uint8_t*)outBuf, strlen(outBuf), 100);
            } while (more == pdTRUE);
            HAL_UART_Transmit(&huart4, (uint8_t*)"HIL> ", 5, 10);
        } else if ((ch == 0x7F || ch == 0x08) && idx > 0) {
            idx--;
            HAL_UART_Transmit(&huart4, (uint8_t*)"\b \b", 3, 10);
        } else if (idx < 127) {
            inBuf[idx++] = (char)ch;
        }
    }
}