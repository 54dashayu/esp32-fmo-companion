/*
 * Copyright (c) 2026 BI8SIG
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file board_config.h
 * @brief 当前硬件板级引脚与外设参数配置。
 *
 * 本文件用于描述当前硬件板子的 GPIO 分配、屏幕分辨率、
 * 触摸校准参数、音频控制引脚、SD 卡 SPI 引脚和电池 ADC 引脚。
 *
 * @note
 * 本文件只放“硬件板级配置”。
 * 应用默认值、功能开关、WebSocket、音频缓冲、省电阈值等配置
 * 应放在 app_config.h。
 *
 * @warning
 * 修改本文件前请确认实际硬件原理图。
 * GPIO 冲突会导致 LCD、触摸、SD、音频或电池检测异常。
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* LCD: M5Stack Core Basic / ILI9342C, driven through ILI9341-compatible path */
/* -------------------------------------------------------------------------- */

/**
 * @brief LCD 水平分辨率。
 */
#define BOARD_LCD_H_RES             320

/**
 * @brief LCD 垂直分辨率。
 */
#define BOARD_LCD_V_RES             240

/**
 * @brief LCD SPI CS 引脚。
 */
#define BOARD_LCD_CS_GPIO           GPIO_NUM_14

/**
 * @brief LCD 数据/命令选择引脚。
 */
#define BOARD_LCD_DC_GPIO           GPIO_NUM_27

/**
 * @brief LCD SPI MOSI 引脚。
 */
#define BOARD_LCD_MOSI_GPIO         GPIO_NUM_23

/**
 * @brief LCD SPI MISO 引脚。
 *
 * 对于只写 LCD，该引脚可能不使用。
 */
#define BOARD_LCD_MISO_GPIO         GPIO_NUM_19

/**
 * @brief LCD SPI SCLK 引脚。
 */
#define BOARD_LCD_SCLK_GPIO         GPIO_NUM_18

/**
 * @brief LCD 背光控制引脚。
 *
 * app_ui.c 中会使用 LEDC PWM 控制该引脚。
 */
#define BOARD_LCD_BL_GPIO           GPIO_NUM_32

/**
 * @brief LCD 复位引脚。
 *
 * GPIO_NUM_NC 表示当前硬件未连接独立复位脚。
 */
#define BOARD_LCD_RST_GPIO          GPIO_NUM_33

/**
 * @brief LCD 背光点亮电平。
 *
 * 1：高电平点亮
 * 0：低电平点亮
 */
#define BOARD_LCD_BL_ON_LEVEL       1

/**
 * @brief LCD 背光关闭电平。
 *
 * 1：高电平关闭
 * 0：低电平关闭
 */
#define BOARD_LCD_BL_OFF_LEVEL      0

/* -------------------------------------------------------------------------- */
/* Input: M5Stack Core front buttons                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief 当前 M5Stack Core Basic 无触摸，使用正面 A/B/C 三个按键。
 */
#define BOARD_HAS_TOUCH             0
#define BOARD_HAS_M5_BUTTONS        1

/**
 * @brief M5Stack Core Button A/B/C GPIO。
 *
 * 按键为低电平按下。
 */
#define BOARD_BUTTON_A_GPIO         GPIO_NUM_39
#define BOARD_BUTTON_B_GPIO         GPIO_NUM_38
#define BOARD_BUTTON_C_GPIO         GPIO_NUM_37
#define BOARD_BUTTON_ACTIVE_LEVEL   0

/**
 * @brief 兼容旧触摸配置引用，M5 Core Basic 未连接触摸。
 */
#define BOARD_TOUCH_CS_GPIO         GPIO_NUM_NC
#define BOARD_TOUCH_CLK_GPIO        GPIO_NUM_NC
#define BOARD_TOUCH_MOSI_GPIO       GPIO_NUM_NC
#define BOARD_TOUCH_MISO_GPIO       GPIO_NUM_NC
#define BOARD_TOUCH_IRQ_GPIO        GPIO_NUM_NC
#define BOARD_TOUCH_IRQ_ACTIVE      0
#define BOARD_TOUCH_X_MIN           0
#define BOARD_TOUCH_X_MAX           0
#define BOARD_TOUCH_Y_MIN           0
#define BOARD_TOUCH_Y_MAX           0

/* -------------------------------------------------------------------------- */
/* Audio output                                                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief ESP32 内置 DAC 音频输出引脚。
 *
 * M5Stack Core Basic 的内置扬声器接 ESP32 DAC GPIO25。
 */
#define BOARD_AUDIO_DAC_GPIO        GPIO_NUM_25

/**
 * @brief 音频功放使能/静音控制引脚。
 */
#define BOARD_AUDIO_EN_GPIO         GPIO_NUM_NC

/**
 * @brief 音频功放使能有效电平。
 *
 * 0：低电平使能
 * 1：高电平使能
 */
#define BOARD_AUDIO_EN_ACTIVE       0

/**
 * @brief 音频功放静音有效电平。
 *
 * 0：低电平静音
 * 1：高电平静音
 */
#define BOARD_AUDIO_MUTE_ACTIVE     1

/* -------------------------------------------------------------------------- */
/* SD card                                                                    */
/* -------------------------------------------------------------------------- */

/**
 * @brief SD 卡 SPI CS 引脚。
 */
#define BOARD_SD_CS_GPIO            GPIO_NUM_4

/**
 * @brief SD 卡 SPI SCLK 引脚。
 */
#define BOARD_SD_SCLK_GPIO          GPIO_NUM_18

/**
 * @brief SD 卡 SPI MISO 引脚。
 */
#define BOARD_SD_MISO_GPIO          GPIO_NUM_19

/**
 * @brief SD 卡 SPI MOSI 引脚。
 */
#define BOARD_SD_MOSI_GPIO          GPIO_NUM_23

/* -------------------------------------------------------------------------- */
/* Battery ADC                                                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief 电池电压 ADC 输入引脚。
 *
 * @note
 * GPIO35 是 ESP32 输入专用 GPIO，适合作为 M5Stack 电池 ADC 输入。
 * 实际电池电压需要结合分压电阻和 app_config.h 中的
 * BATTERY_VOLTAGE_SCALE_PERMILLE 计算。
 */
#define BOARD_BAT_ADC_GPIO          GPIO_NUM_35

/* -------------------------------------------------------------------------- */
/* RGB side LED strip: M5Stack Core / Fire                                    */
/* -------------------------------------------------------------------------- */

/**
 * @brief M5 Core / Fire 侧边 RGB 灯带使能。
 *
 * Fire/Core 带灯带的机型通常使用 GPIO15 驱动 10 颗 WS2812/SK6812 LED。
 * M5 Fire / 带侧边灯带机型默认启用。
 * 不带灯带的 M5 Core Basic 可将 BOARD_HAS_RGB_LED_STRIP 改为 0。
 */
#define BOARD_HAS_RGB_LED_STRIP     1
#define BOARD_RGB_LED_GPIO          GPIO_NUM_15
#define BOARD_RGB_LED_COUNT         10

#ifdef __cplusplus
}
#endif

#endif /* BOARD_CONFIG_H */
