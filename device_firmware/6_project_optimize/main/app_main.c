/* ESP32-C3 Light Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_params.h"
#include "esp_rmaker_standard_devices.h"
#include "esp_rmaker_ota.h"
#include "esp_rmaker_schedule.h"

#include "app_wifi.h"
#include "app_storage.h"
#include "app_priv.h"

static const char *TAG = "performance_optimize";

esp_rmaker_device_t *light_device;

extern const char ota_server_cert[] asm("_binary_server_crt_start");

/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", device_name, param_name);
        app_light_set_power(val.val.b);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        app_light_set_brightness(val.val.i);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        app_light_set_hue(val.val.i);
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        app_light_set_saturation(val.val.i);
    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}
#include "esp_pm.h"


void app_main()
{    
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG, "app_main start");

    // NVS init 
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");

    app_storage_init();

    ESP_LOGI(TAG, "Init PM");
    app_pm_init();

    ESP_LOGI(TAG, "App driver init");
    app_driver_init();

    ESP_LOGI(TAG, "WiFi init");
    app_wifi_init();

    ESP_LOGI(TAG, "About to init RainMaker node");
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "RM_Device", "Light");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(pdMS_TO_TICKS(5000));
        abort();
    }

    /* Create a device and add the relevant parameters to it */
    light_device = esp_rmaker_lightbulb_device_create("Light", NULL, DEFAULT_POWER);
    esp_rmaker_device_add_cb(light_device, write_cb, NULL);

    esp_rmaker_device_add_param(light_device, esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME, DEFAULT_BRIGHTNESS));
    esp_rmaker_device_add_param(light_device, esp_rmaker_hue_param_create(ESP_RMAKER_DEF_HUE_NAME, DEFAULT_HUE));
    esp_rmaker_device_add_param(light_device, esp_rmaker_saturation_param_create(ESP_RMAKER_DEF_SATURATION_NAME, DEFAULT_SATURATION));

    esp_rmaker_node_add_device(node, light_device);

    /* Enable OTA */
    esp_rmaker_ota_config_t ota_config = {
        .server_cert = ota_server_cert,
    };
    esp_rmaker_ota_enable(&ota_config, OTA_USING_PARAMS);

    esp_rmaker_timezone_service_enable();
    esp_rmaker_schedule_enable();

    ESP_LOGI(TAG, "Starting esp_rmaker");
    esp_rmaker_start();
    ESP_LOGI(TAG, "esp_rmaker_start returned");

    ESP_LOGI(TAG, "Starting WiFi");
    err = app_wifi_start(POP_TYPE_RANDOM);
    ESP_LOGI(TAG, "app_wifi_start returned: %d", err);

    // Wait for WiFi and RainMaker to stabilize
    ESP_LOGI(TAG, "Waiting for system to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(10000)); // 10 seconds
    
    // Initial check right after WiFi connects
    // ESP_LOGI(TAG, "=== Initial Check (10s after WiFi) ===");
    // esp_pm_dump_locks(stdout);
    // app_wifi_check_power_save();

    // NOW create monitoring task to check over time
    // ESP_LOGI(TAG, "Creating PM monitoring task");
    
    // xTaskCreate(pm_monitoring_task, "pm_monitor", 4096, NULL, 5, NULL);
    while (1){
        esp_pm_dump_locks(stdout);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

}