/*
 * Copyright (c) 2026 BI8SIG
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file app_profile_import.c
 * @brief 从 M5 Core TF 卡导入连接配置。
 */

#include "app_profile_import.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "app_settings.h"

static const char *TAG = "profile_import";

#define SD_MOUNT_POINT              "/sdcard"
#define PROFILE_IMPORT_CSV_PATH     SD_MOUNT_POINT "/fmo_profiles.csv"

#define M5_SD_SPI_HOST              SPI2_HOST
#define M5_SD_PIN_MOSI              GPIO_NUM_23
#define M5_SD_PIN_MISO              GPIO_NUM_19
#define M5_SD_PIN_CLK               GPIO_NUM_18
#define M5_SD_PIN_CS                GPIO_NUM_4

#define IMPORT_LINE_MAX             320
#define IMPORT_FIELD_MAX            8

static char *trim(char *s);
static bool parse_bool_field(const char *value);
static bool parse_u8_slot(const char *value, uint8_t *slot);
static bool line_is_blank_or_comment(const char *line);
static int parse_csv_line(char *line, char *fields[], int max_fields);
static bool copy_field(char *dst, size_t dst_size, const char *src);
static bool profile_name_is_ascii(const char *src);
static esp_err_t import_profiles_file(const char *path);
static esp_err_t mount_sdcard(sdmmc_card_t **card, bool *bus_initialized);
static void unmount_sdcard(sdmmc_card_t *card, bool bus_initialized);

esp_err_t app_profile_import_from_sdcard(void)
{
    sdmmc_card_t *card = NULL;
    bool bus_initialized = false;

    esp_err_t err = mount_sdcard(&card, &bus_initialized);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "sdcard not available: %s", esp_err_to_name(err));
        return err;
    }

    err = import_profiles_file(PROFILE_IMPORT_CSV_PATH);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "profile import finished");
    } else if (err == ESP_ERR_NOT_FOUND) {
        ESP_LOGI(TAG, "profile import file not found");
    } else {
        ESP_LOGW(TAG, "profile import failed: %s", esp_err_to_name(err));
    }

    unmount_sdcard(card, bus_initialized);

    return err;
}

static esp_err_t mount_sdcard(sdmmc_card_t **card, bool *bus_initialized)
{
    if (!card || !bus_initialized) {
        return ESP_ERR_INVALID_ARG;
    }

    *card = NULL;
    *bus_initialized = false;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = M5_SD_SPI_HOST;
    host.max_freq_khz = 10000;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = M5_SD_PIN_MOSI,
        .miso_io_num = M5_SD_PIN_MISO,
        .sclk_io_num = M5_SD_PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t err = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (err == ESP_OK) {
        *bus_initialized = true;
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "spi bus already initialized, reuse it");
    } else {
        return err;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = host.slot;
    slot_config.gpio_cs = M5_SD_PIN_CS;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 2,
        .allocation_unit_size = 0,
    };

    err = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT,
                                  &host,
                                  &slot_config,
                                  &mount_config,
                                  card);
    if (err != ESP_OK && *bus_initialized) {
        spi_bus_free(host.slot);
        *bus_initialized = false;
    }

    return err;
}

static void unmount_sdcard(sdmmc_card_t *card, bool bus_initialized)
{
    if (card) {
        esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, card);
    }

    if (bus_initialized) {
        spi_bus_free(M5_SD_SPI_HOST);
    }
}

static esp_err_t import_profiles_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return ESP_ERR_NOT_FOUND;
    }

    app_settings_t cfg;
    esp_err_t err = app_settings_load(&cfg);
    if (err != ESP_OK) {
        fclose(fp);
        return err;
    }

    for (uint8_t i = 0; i < APP_CONNECTION_PROFILE_MAX; i++) {
        memset(&cfg.connection_profiles[i], 0, sizeof(cfg.connection_profiles[i]));
        snprintf(cfg.connection_profiles[i].name,
                 sizeof(cfg.connection_profiles[i].name),
                 "配置%u",
                 (unsigned)(i + 1));
    }

    bool imported_any = false;
    bool active_seen = false;
    bool owner_callsign_seen = false;
    uint8_t active_index = cfg.active_profile_index;
    char line[IMPORT_LINE_MAX];
    unsigned line_no = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_no++;

        char *start = trim(line);
        if (line_is_blank_or_comment(start)) {
            continue;
        }

        char line_copy[IMPORT_LINE_MAX];
        strlcpy(line_copy, start, sizeof(line_copy));

        char *fields[IMPORT_FIELD_MAX] = {0};
        int field_count = parse_csv_line(line_copy, fields, IMPORT_FIELD_MAX);
        if (field_count <= 0) {
            continue;
        }

        int slot_field = 0;
        uint8_t slot = 0;
        if (!parse_u8_slot(fields[slot_field], &slot)) {
            slot_field = 1;
            if (field_count < 8 || !parse_u8_slot(fields[slot_field], &slot)) {
                if (!imported_any &&
                    (strcasecmp(fields[0], "slot") == 0 ||
                     strcasecmp(fields[0], "owner_callsign") == 0)) {
                    ESP_LOGI(TAG, "skip header line");
                    continue;
                }

                if (line_no == 1) {
                    ESP_LOGI(TAG, "skip header line");
                    continue;
                }

                ESP_LOGW(TAG, "line %u invalid slot", line_no);
                fclose(fp);
                return ESP_ERR_INVALID_ARG;
            }
        }

        if (!owner_callsign_seen &&
            slot_field == 1 &&
            fields[0] &&
            fields[0][0] != '\0') {
            if (!copy_field(cfg.owner_callsign,
                            sizeof(cfg.owner_callsign),
                            fields[0])) {
                ESP_LOGW(TAG, "line %u owner callsign too long", line_no);
                fclose(fp);
                return ESP_ERR_INVALID_SIZE;
            }
            owner_callsign_seen = true;
        }

        int name_field = slot_field + 1;
        int ssid_field = slot_field + 2;
        int password_field = slot_field + 3;
        int host_field = slot_field + 4;
        int ddns_field = slot_field + 5;
        int active_field = slot_field + 6;

        uint8_t index = (uint8_t)(slot - 1);
        app_connection_profile_t *profile = &cfg.connection_profiles[index];

        if (field_count > name_field &&
            fields[name_field] &&
            fields[name_field][0] != '\0') {
            if (!profile_name_is_ascii(fields[name_field])) {
                ESP_LOGW(TAG,
                         "line %u profile name has unsupported glyphs, keep default",
                         line_no);
            } else {
                if (!copy_field(profile->name,
                                sizeof(profile->name),
                                fields[name_field])) {
                    ESP_LOGW(TAG, "line %u profile name too long", line_no);
                    fclose(fp);
                    return ESP_ERR_INVALID_SIZE;
                }
            }
        }

        if (field_count > ssid_field && fields[ssid_field]) {
            if (!copy_field(profile->wifi_ssid,
                            sizeof(profile->wifi_ssid),
                            fields[ssid_field])) {
                ESP_LOGW(TAG, "line %u wifi ssid too long", line_no);
                fclose(fp);
                return ESP_ERR_INVALID_SIZE;
            }
        }

        if (field_count > password_field && fields[password_field]) {
            if (!copy_field(profile->wifi_password,
                            sizeof(profile->wifi_password),
                            fields[password_field])) {
                ESP_LOGW(TAG, "line %u wifi password too long", line_no);
                fclose(fp);
                return ESP_ERR_INVALID_SIZE;
            }
        }

        if (field_count > host_field && fields[host_field]) {
            if (!copy_field(profile->fmo_host,
                            sizeof(profile->fmo_host),
                            fields[host_field])) {
                ESP_LOGW(TAG, "line %u fmo host too long", line_no);
                fclose(fp);
                return ESP_ERR_INVALID_SIZE;
            }
        }

        if (field_count > ddns_field && fields[ddns_field]) {
            profile->ddns_remote_enabled = parse_bool_field(fields[ddns_field]);
        }

        if (field_count > active_field &&
            fields[active_field] &&
            parse_bool_field(fields[active_field])) {
            active_index = index;
            active_seen = true;
        }

        imported_any = true;
    }

    fclose(fp);

    if (!imported_any) {
        return ESP_ERR_NOT_FOUND;
    }

    if (!active_seen) {
        for (uint8_t i = 0; i < APP_CONNECTION_PROFILE_MAX; i++) {
            if (cfg.connection_profiles[i].wifi_ssid[0] != '\0' &&
                cfg.connection_profiles[i].fmo_host[0] != '\0') {
                active_index = i;
                active_seen = true;
                break;
            }
        }
    }

    if (!active_seen ||
        cfg.connection_profiles[active_index].wifi_ssid[0] == '\0' ||
        cfg.connection_profiles[active_index].fmo_host[0] == '\0') {
        ESP_LOGW(TAG, "no usable active profile in import file");
        return ESP_ERR_INVALID_STATE;
    }

    cfg.active_profile_index = active_index;

    err = app_settings_save(&cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG,
                 "imported profiles from %s, active=%u",
                 path,
                 (unsigned)(active_index + 1));
    }

    return err;
}

static char *trim(char *s)
{
    if (!s) {
        return s;
    }

    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *end = '\0';

    return s;
}

static bool line_is_blank_or_comment(const char *line)
{
    return !line || line[0] == '\0' || line[0] == '#';
}

static bool parse_u8_slot(const char *value, uint8_t *slot)
{
    if (!value || !slot) {
        return false;
    }

    char *end = NULL;
    long parsed = strtol(value, &end, 10);
    if (end == value || *trim(end) != '\0') {
        return false;
    }

    if (parsed < 1 || parsed > APP_CONNECTION_PROFILE_MAX) {
        return false;
    }

    *slot = (uint8_t)parsed;
    return true;
}

static bool parse_bool_field(const char *value)
{
    if (!value) {
        return false;
    }

    char tmp[8];
    strlcpy(tmp, value, sizeof(tmp));
    char *v = trim(tmp);

    for (char *p = v; *p; p++) {
        *p = (char)tolower((unsigned char)*p);
    }

    return strcmp(v, "1") == 0 ||
           strcmp(v, "y") == 0 ||
           strcmp(v, "yes") == 0 ||
           strcmp(v, "true") == 0 ||
           strcmp(v, "on") == 0;
}

static int parse_csv_line(char *line, char *fields[], int max_fields)
{
    if (!line || !fields || max_fields <= 0) {
        return 0;
    }

    int count = 0;
    char *read = line;
    char *write = line;
    bool in_quotes = false;
    fields[count++] = write;

    while (*read && count <= max_fields) {
        char c = *read++;

        if (c == '"') {
            if (in_quotes && *read == '"') {
                *write++ = *read++;
            } else {
                in_quotes = !in_quotes;
            }
            continue;
        }

        if (!in_quotes && c == ',') {
            *write++ = '\0';
            if (count >= max_fields) {
                break;
            }
            fields[count++] = write;
            continue;
        }

        *write++ = c;
    }

    *write = '\0';

    for (int i = 0; i < count; i++) {
        fields[i] = trim(fields[i]);
    }

    return count;
}

static bool copy_field(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0 || !src) {
        return false;
    }

    size_t len = strlen(src);
    if (len >= dst_size) {
        return false;
    }

    memset(dst, 0, dst_size);
    memcpy(dst, src, len);
    dst[len] = '\0';

    return true;
}

static bool profile_name_is_ascii(const char *src)
{
    if (!src) {
        return false;
    }

    for (const unsigned char *p = (const unsigned char *)src; *p; p++) {
        if (*p < 0x20 || *p > 0x7e) {
            return false;
        }
    }

    return true;
}
