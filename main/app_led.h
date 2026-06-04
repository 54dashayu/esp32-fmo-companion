/*
 * Copyright (c) 2026 BI8SIG
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file app_led.h
 * @brief M5 Core / Fire 侧边 RGB 灯带状态提醒接口。
 */

#ifndef APP_LED_H
#define APP_LED_H

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_LED_CALL_ALERT_NEVER = 0,
    APP_LED_CALL_ALERT_NOT_TODAY,
    APP_LED_CALL_ALERT_RECENT_15_MIN,
} app_led_call_alert_t;

esp_err_t app_led_init(void);

void app_led_set_qso_state(const char *callsign, bool speaking);
void app_led_notify_callsign(app_led_call_alert_t alert);
void app_led_set_power_save(bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* APP_LED_H */
