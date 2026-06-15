/*
 * Copyright (c) 2026 BI8SIG
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file app_profile_import.h
 * @brief 从 M5 Core TF 卡导入连接配置。
 */

#ifndef APP_PROFILE_IMPORT_H
#define APP_PROFILE_IMPORT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 尝试从 TF 卡根目录导入 5 组连接配置。
 *
 * 支持的文件名：
 * - /sdcard/fmo_profiles.csv
 *
 * 未插卡、挂载失败或文件不存在时返回错误码，不覆盖原配置。
 */
esp_err_t app_profile_import_from_sdcard(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PROFILE_IMPORT_H */
