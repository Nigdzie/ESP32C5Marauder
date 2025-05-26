#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "driver/uart.h"

#define TAG "WiFiScanner"
#define CLI_UART_BUF_SIZE 128
#define CLI_UART_PORT UART_NUM_0

typedef struct {
    uint8_t bssid[6];
    uint8_t channel;
    int rssi;
    char ssid[33];
} ap_info_t;

static ap_info_t *ap_list = NULL;
static int ap_list_count = 0;

static volatile bool scanap_running = false;
static TaskHandle_t scanap_task_handle = NULL;

const char* get_band_from_channel(uint8_t ch) {
    if (ch >= 1 && ch <= 14)
        return "2.4G";
    else if (ch >= 36 && ch <= 165)
        return "5G";
    else if (ch >= 1 && ch <= 233)
        return "6G";
    return "?";
}

// Scan all APs dynamically (no limit)
void wifi_scan_aps_only(void) {
    wifi_scan_config_t scan_config = {
        .ssid = 0, .bssid = 0, .channel = 0, .show_hidden = true
    };
    esp_wifi_set_promiscuous(false);
    esp_wifi_scan_start(&scan_config, true);

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    if (ap_list) {
        free(ap_list);
        ap_list = NULL;
    }

    if (ap_count == 0) {
        ap_list_count = 0;
        return;
    }

    wifi_ap_record_t *scan_results = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (!scan_results) {
        ap_list_count = 0;
        return;
    }

    if (esp_wifi_scan_get_ap_records(&ap_count, scan_results) != ESP_OK) {
        free(scan_results);
        ap_list_count = 0;
        return;
    }

    ap_list = malloc(sizeof(ap_info_t) * ap_count);
    if (!ap_list) {
        free(scan_results);
        ap_list_count = 0;
        return;
    }

    ap_list_count = ap_count;
    for (int i = 0; i < ap_list_count; i++) {
        memcpy(ap_list[i].bssid, scan_results[i].bssid, 6);
        ap_list[i].channel = scan_results[i].primary;
        ap_list[i].rssi = scan_results[i].rssi;
        memcpy(ap_list[i].ssid, scan_results[i].ssid, sizeof(ap_list[i].ssid));
        ap_list[i].ssid[32] = '\0';
    }

    free(scan_results);
}

void print_ap_list_flipper_band(void) {
    for (int i = 0; i < ap_list_count; i++) {
        printf("Band: %s RSSI: %d Ch: %d BSSID: %02x:%02x:%02x:%02x:%02x:%02x ESSID: %s\n",
            get_band_from_channel(ap_list[i].channel),
            ap_list[i].rssi,
            ap_list[i].channel,
            ap_list[i].bssid[0], ap_list[i].bssid[1], ap_list[i].bssid[2],
            ap_list[i].bssid[3], ap_list[i].bssid[4], ap_list[i].bssid[5],
            ap_list[i].ssid[0] ? ap_list[i].ssid : "<hidden>"
        );
    }
}

void scanap_loop_task(void *pvParameters) {
    while (scanap_running) {
        wifi_scan_aps_only();
        print_ap_list_flipper_band();
        vTaskDelay(pdMS_TO_TICKS(1700));
    }
    scanap_task_handle = NULL;
    vTaskDelete(NULL);
}

static void sanitize_command(char* dst, const uint8_t* src, size_t maxlen) {
    size_t pos = 0;
    for (size_t i = 0; src[i] && pos < maxlen - 1; ++i) {
        if (isalnum(src[i]) || src[i] == '_' || src[i] == '-' || src[i] == '.') {
            dst[pos++] = src[i];
        }
    }
    dst[pos] = '\0';
}

void cli_task(void *pvParameters) {
    uint8_t data[CLI_UART_BUF_SIZE];
    char command[CLI_UART_BUF_SIZE];
    int pos = 0;
    printf("\nESP32 Marauder CLI ready. Type 'help'.\n> ");
    fflush(stdout);

    while (1) {
        int len = uart_read_bytes(CLI_UART_PORT, &data[pos], 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            uint8_t ch = data[pos];
            if (ch == '\r' || ch == '\n') {
                data[pos] = 0;
                sanitize_command(command, data, CLI_UART_BUF_SIZE);

                if (strlen(command) >= 3) {
                    if (strcmp(command, "scanap") == 0) {
                        if (!scanap_running) {
                            scanap_running = true;
                            xTaskCreate(scanap_loop_task, "scanap_loop", 4096, NULL, 5, &scanap_task_handle);
                            printf("Continuous scan started (scanap).\n");
                        } else {
                            printf("Scan already running.\n");
                        }
                    } else if (strcmp(command, "stopscan") == 0) {
                        scanap_running = false;
                        printf("Scan stopped.\n");
                    } else if (strcmp(command, "help") == 0) {
                        printf("Available commands:\n");
                        printf("  scanap   - Continuous AP scan\n");
                        printf("  stopscan - Stop AP scan\n");
                        printf("  help     - Show this help\n");
                    } else {
                        printf("Unknown command: '%s'. Type 'help' for list of commands.\n", command);
                    }
                }
                pos = 0;
                printf("> ");
                fflush(stdout);
                continue;
            }
            if (ch >= 32 && ch < 127 && pos < CLI_UART_BUF_SIZE - 1) {
                uart_write_bytes(CLI_UART_PORT, (const char*)&ch, 1); // echo
                pos++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_WARN);

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(CLI_UART_PORT, 2048, 0, 0, NULL, 0);
    uart_param_config(CLI_UART_PORT, &uart_config);
    uart_set_pin(CLI_UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(cli_task, "cli_task", 4096, NULL, 5, NULL);
}
