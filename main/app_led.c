/*
 * Copyright (c) 2026 BI8SIG
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file app_led.c
 * @brief M5 Core / Fire 侧边 RGB 灯带状态提醒实现。
 */

#include "app_led.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"

#include "app_config.h"
#include "app_settings.h"
#include "board_config.h"

static const char *TAG = "app_led";

#define APP_LED_TASK_PERIOD_MS       30
#define APP_LED_ALERT_DURATION_US    2000000LL
#define APP_LED_STEADY_BRIGHTNESS    36
#define APP_LED_ALERT_BRIGHTNESS     90
#define APP_LED_CALLSIGN_MAX_LEN     16

typedef enum {
    APP_LED_MODE_STEADY = 0,
    APP_LED_MODE_RAINBOW,
    APP_LED_MODE_ORANGE_BREATHE,
    APP_LED_MODE_BLUE_BREATHE,
} app_led_mode_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} app_led_rgb_t;

#if BOARD_HAS_RGB_LED_STRIP
static led_strip_handle_t s_led_strip = NULL;
static TaskHandle_t s_led_task_handle = NULL;
static portMUX_TYPE s_led_mux = portMUX_INITIALIZER_UNLOCKED;

static bool s_speaking = false;
static bool s_owner_speaking = false;
static bool s_power_save = false;
static app_led_mode_t s_alert_mode = APP_LED_MODE_STEADY;
static int64_t s_alert_until_us = 0;

static void app_led_task(void *arg);
static void app_led_render(int64_t now_us);
static void app_led_set_all(app_led_rgb_t color);
static void app_led_set_pixel(int index, app_led_rgb_t color);
static void app_led_clear(void);
static app_led_rgb_t app_led_scale(app_led_rgb_t color, uint8_t brightness);
static app_led_rgb_t app_led_hsv(uint16_t hue, uint8_t sat, uint8_t val);
static uint8_t app_led_breathe_value(int64_t now_us, uint8_t max_value);
static bool app_led_callsign_is_owner(const char *callsign);
#endif

esp_err_t app_led_init(void)
{
#if BOARD_HAS_RGB_LED_STRIP
    if (s_led_strip) {
        return ESP_OK;
    }

    led_strip_config_t strip_config = {
        .strip_gpio_num = BOARD_RGB_LED_GPIO,
        .max_leds = BOARD_RGB_LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags = {
            .with_dma = false,
        },
    };

    esp_err_t ret = led_strip_new_rmt_device(&strip_config,
                                             &rmt_config,
                                             &s_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "led strip init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    app_led_clear();

    BaseType_t task_ret = xTaskCreate(app_led_task,
                                      "app_led",
                                      3072,
                                      NULL,
                                      3,
                                      &s_led_task_handle);
    if (task_ret != pdPASS) {
        led_strip_del(s_led_strip);
        s_led_strip = NULL;
        ESP_LOGW(TAG, "create app_led task failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG,
             "RGB LED strip ready, gpio=%d count=%d",
             BOARD_RGB_LED_GPIO,
             BOARD_RGB_LED_COUNT);

    return ESP_OK;
#else
    ESP_LOGI(TAG, "RGB LED strip disabled by board config");
    return ESP_OK;
#endif
}

void app_led_set_qso_state(const char *callsign, bool speaking)
{
#if BOARD_HAS_RGB_LED_STRIP
    bool owner = speaking && app_led_callsign_is_owner(callsign);

    portENTER_CRITICAL(&s_led_mux);
    if (!s_power_save) {
        s_speaking = speaking;
        s_owner_speaking = owner;
    }
    portEXIT_CRITICAL(&s_led_mux);
#else
    (void)callsign;
    (void)speaking;
#endif
}

void app_led_notify_callsign(app_led_call_alert_t alert)
{
#if BOARD_HAS_RGB_LED_STRIP
    app_led_mode_t mode = APP_LED_MODE_BLUE_BREATHE;

    if (alert == APP_LED_CALL_ALERT_NEVER) {
        mode = APP_LED_MODE_RAINBOW;
    } else if (alert == APP_LED_CALL_ALERT_NOT_TODAY) {
        mode = APP_LED_MODE_ORANGE_BREATHE;
    }

    portENTER_CRITICAL(&s_led_mux);
    if (!s_power_save) {
        s_alert_mode = mode;
        s_alert_until_us = esp_timer_get_time() + APP_LED_ALERT_DURATION_US;
    }
    portEXIT_CRITICAL(&s_led_mux);
#else
    (void)alert;
#endif
}

void app_led_set_power_save(bool enabled)
{
#if BOARD_HAS_RGB_LED_STRIP
    portENTER_CRITICAL(&s_led_mux);
    s_power_save = enabled;
    if (enabled) {
        s_speaking = false;
        s_owner_speaking = false;
        s_alert_mode = APP_LED_MODE_STEADY;
        s_alert_until_us = 0;
    }
    portEXIT_CRITICAL(&s_led_mux);

    if (enabled) {
        app_led_clear();
    }
#else
    (void)enabled;
#endif
}

#if BOARD_HAS_RGB_LED_STRIP
static void app_led_task(void *arg)
{
    (void)arg;

    while (true) {
        app_led_render(esp_timer_get_time());
        vTaskDelay(pdMS_TO_TICKS(APP_LED_TASK_PERIOD_MS));
    }
}

static void app_led_render(int64_t now_us)
{
    if (!s_led_strip) {
        return;
    }

    bool speaking;
    bool owner_speaking;
    bool power_save;
    app_led_mode_t alert_mode;
    int64_t alert_until_us;

    portENTER_CRITICAL(&s_led_mux);
    speaking = s_speaking;
    owner_speaking = s_owner_speaking;
    power_save = s_power_save;
    alert_mode = s_alert_mode;
    alert_until_us = s_alert_until_us;

    if (alert_mode != APP_LED_MODE_STEADY && now_us >= alert_until_us) {
        s_alert_mode = APP_LED_MODE_STEADY;
        alert_mode = APP_LED_MODE_STEADY;
    }
    portEXIT_CRITICAL(&s_led_mux);

    if (power_save) {
        app_led_clear();
        return;
    }

    if (alert_mode == APP_LED_MODE_RAINBOW) {
        uint16_t phase = (uint16_t)((now_us / 7000) % 360);
        uint8_t breath = app_led_breathe_value(now_us, APP_LED_ALERT_BRIGHTNESS);

        for (int i = 0; i < BOARD_RGB_LED_COUNT; i++) {
            uint16_t hue = (uint16_t)((phase + i * 360 / BOARD_RGB_LED_COUNT) % 360);
            app_led_set_pixel(i, app_led_hsv(hue, 255, breath));
        }
        led_strip_refresh(s_led_strip);
        return;
    }

    if (alert_mode == APP_LED_MODE_ORANGE_BREATHE) {
        uint8_t breath = app_led_breathe_value(now_us, APP_LED_ALERT_BRIGHTNESS);
        app_led_set_all(app_led_scale((app_led_rgb_t){255, 140, 0}, breath));
        return;
    }

    if (alert_mode == APP_LED_MODE_BLUE_BREATHE) {
        uint8_t breath = app_led_breathe_value(now_us, APP_LED_ALERT_BRIGHTNESS);
        app_led_set_all(app_led_scale((app_led_rgb_t){0, 120, 255}, breath));
        return;
    }

    if (speaking) {
        app_led_rgb_t color = owner_speaking ?
            (app_led_rgb_t){255, 0, 0} :
            (app_led_rgb_t){0, 255, 0};
        app_led_set_all(app_led_scale(color, APP_LED_STEADY_BRIGHTNESS));
    } else {
        app_led_clear();
    }
}

static void app_led_set_all(app_led_rgb_t color)
{
    if (!s_led_strip) {
        return;
    }

    for (int i = 0; i < BOARD_RGB_LED_COUNT; i++) {
        app_led_set_pixel(i, color);
    }

    led_strip_refresh(s_led_strip);
}

static void app_led_set_pixel(int index, app_led_rgb_t color)
{
    led_strip_set_pixel(s_led_strip, index, color.r, color.g, color.b);
}

static void app_led_clear(void)
{
    if (!s_led_strip) {
        return;
    }

    led_strip_clear(s_led_strip);
}

static app_led_rgb_t app_led_scale(app_led_rgb_t color, uint8_t brightness)
{
    app_led_rgb_t out = {
        .r = (uint8_t)((uint16_t)color.r * brightness / 255),
        .g = (uint8_t)((uint16_t)color.g * brightness / 255),
        .b = (uint8_t)((uint16_t)color.b * brightness / 255),
    };

    return out;
}

static app_led_rgb_t app_led_hsv(uint16_t hue, uint8_t sat, uint8_t val)
{
    uint8_t region = hue / 60;
    uint16_t remainder = (hue - region * 60) * 255 / 60;

    uint8_t p = (uint8_t)((uint16_t)val * (255 - sat) / 255);
    uint8_t q = (uint8_t)((uint16_t)val * (255 - ((uint16_t)sat * remainder / 255)) / 255);
    uint8_t t = (uint8_t)((uint16_t)val * (255 - ((uint16_t)sat * (255 - remainder) / 255)) / 255);

    switch (region) {
    case 0:
        return (app_led_rgb_t){val, t, p};
    case 1:
        return (app_led_rgb_t){q, val, p};
    case 2:
        return (app_led_rgb_t){p, val, t};
    case 3:
        return (app_led_rgb_t){p, q, val};
    case 4:
        return (app_led_rgb_t){t, p, val};
    default:
        return (app_led_rgb_t){val, p, q};
    }
}

static uint8_t app_led_breathe_value(int64_t now_us, uint8_t max_value)
{
    int64_t phase_us = now_us % 1000000LL;
    int32_t half = (phase_us < 500000LL) ?
        (int32_t)phase_us :
        (int32_t)(1000000LL - phase_us);

    uint8_t value = (uint8_t)((int32_t)max_value * half / 500000);
    if (value < 4) {
        value = 4;
    }

    return value;
}

static bool app_led_callsign_is_owner(const char *callsign)
{
    if (!callsign || callsign[0] == '\0') {
        return false;
    }

    const app_settings_t *cfg = app_settings_get();
    const char *owner = (cfg && cfg->owner_callsign[0]) ?
        cfg->owner_callsign :
        APP_DEFAULT_OWNER_CALLSIGN;

    char call_buf[APP_LED_CALLSIGN_MAX_LEN];
    char owner_buf[APP_LED_CALLSIGN_MAX_LEN];

    snprintf(call_buf, sizeof(call_buf), "%s", callsign);
    snprintf(owner_buf, sizeof(owner_buf), "%s", owner);

    for (size_t i = 0; call_buf[i]; i++) {
        if (call_buf[i] >= 'a' && call_buf[i] <= 'z') {
            call_buf[i] = (char)(call_buf[i] - 'a' + 'A');
        }
    }

    for (size_t i = 0; owner_buf[i]; i++) {
        if (owner_buf[i] >= 'a' && owner_buf[i] <= 'z') {
            owner_buf[i] = (char)(owner_buf[i] - 'a' + 'A');
        }
    }

    return strcmp(call_buf, owner_buf) == 0;
}
#endif
