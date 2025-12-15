/* ESP32-C3 Light Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

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
#include "app_insights.h"

#include "esp_diagnostics_metrics.h" //
#include "esp_diagnostics_variables.h" //

#include "esp_pm.h"
static const char *TAG = "rainmaker_insight";
esp_rmaker_device_t *light_device;
//
static struct {
    bool power;
    int brightness;
    int hue;
    int saturation;
} light_state = {
    .power = DEFAULT_POWER,
    .brightness = DEFAULT_BRIGHTNESS,
    .hue = DEFAULT_HUE,
    .saturation = DEFAULT_SATURATION
};
//
// add
/* Initialize light metrics for Insights dashboard */
static void app_light_metrics_init(void)
{
    esp_diag_metrics_register("light", "power", "Light Power", "light.power", ESP_DIAG_DATA_TYPE_BOOL);

    esp_diag_metrics_register("light", "brightness", "Light Brightness", "light.brightness", 
        ESP_DIAG_DATA_TYPE_UINT);

    esp_diag_metrics_register("light", "hue", "Light Hue", "light.hue", ESP_DIAG_DATA_TYPE_UINT);

    esp_diag_metrics_register("light", "saturation", "Light Saturation", "light.saturation", 
        ESP_DIAG_DATA_TYPE_UINT);    
   
    ESP_LOGI(TAG, "Light metrics initialized");
}

static void app_light_report_metrics(void)
{
    esp_diag_metrics_add_bool("power", light_state.power);

    uint32_t brightness_val = (uint32_t)light_state.brightness;
    esp_diag_metrics_add_uint("brightness", brightness_val);

    uint32_t hue_val = (uint32_t)light_state.hue;
    esp_diag_metrics_add_uint("hue", hue_val);

    uint32_t saturation_val = (uint32_t)light_state.saturation;
    esp_diag_metrics_add_uint("saturation", saturation_val);
}

//



extern const char ota_server_cert[] asm("_binary_server_crt_start");


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
        
        // Cập nhật trạng thái
        light_state.power = val.val.b;
        app_light_set_power(val.val.b);
        
        // Report metric to Insights
        esp_diag_metrics_add_bool("power", light_state.power);
        
    } else if (strcmp(param_name, ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        
        // Cập nhật trạng thái
        light_state.brightness = val.val.i;
        app_light_set_brightness(val.val.i);
        
        // Report metric to Insights
        uint32_t brightness = (uint32_t)light_state.brightness;
        esp_diag_metrics_add_uint("brightness", brightness);
        
    } else if (strcmp(param_name, ESP_RMAKER_DEF_HUE_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        
        // Cập nhật trạng thái
        light_state.hue = val.val.i;
        app_light_set_hue(val.val.i);
        
        // Report metric to Insights
        uint32_t hue = (uint32_t)light_state.hue;
        esp_diag_metrics_add_uint("hue", hue);
        
    } else if (strcmp(param_name, ESP_RMAKER_DEF_SATURATION_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %d for %s - %s",
                val.val.i, device_name, param_name);
        
        // Cập nhật trạng thái
        light_state.saturation = val.val.i;
        app_light_set_saturation(val.val.i);
        
        // Report metric to Insights
        uint32_t saturation = (uint32_t)light_state.saturation;
        esp_diag_metrics_add_uint("saturation", saturation);
        
    } else {
        /* Silently ignoring invalid params */
        return ESP_OK;
    }
    esp_rmaker_param_update_and_report(param, val);
    return ESP_OK;
}

void vCrashTask (void *pvParameters){
    vTaskDelay(pdMS_TO_TICKS(10000));
    printf("CRASH TEST: Null pointer\n");
    volatile int *ptr = NULL;
    *ptr = 10;
}

void app_main()
{
    // int i = 0;
    esp_err_t err = ESP_OK;
    ESP_LOGE(TAG, "app_main");
    // NVS init 
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    /**
     * @brief NVS Flash initialization
     */
    ESP_LOGI(TAG, "NVS Flash initialization");
    app_storage_init();

    /**
     * @brief Power Manager initialization
     */
    app_pm_init();
    
    /**
     * @brief Application driver initialization
     */
    ESP_LOGI(TAG, "Application driver initialization");
    app_driver_init();

    /**
     * @brief Initialize Wi-Fi. Note that, this should be called before esp_rmaker_init()
     */
    app_wifi_init();
    
    /**
     * @brief Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "ESP RainMaker Device", "Lightbulb");
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

    /* Enable timezone service which will be require for setting appropriate timezone
     * from the phone apps for scheduling to work correctly.
     * For more information on the various ways of setting timezone, please check
     * https://rainmaker.espressif.com/docs/time-service.html.
     */
    esp_rmaker_timezone_service_enable();

    /* Enable scheduling. */
    esp_rmaker_schedule_enable();

    /* Enable Insights. Requires CONFIG_ESP_INSIGHTS_ENABLED=y */
    app_insights_enable();
    /* Initialize light metrics */
    app_light_metrics_init(); //

    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */
    err = app_wifi_start(POP_TYPE_RANDOM);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(pdMS_TO_TICKS(5000));
        abort();
    }

    /* Report initial light values to Insights */
    app_light_report_metrics(); //
    // xTaskCreate(vCrashTask, "crashTask", 2048, NULL, 1, NULL);
    while (1){
        esp_pm_dump_locks(stdout);
        app_light_report_metrics(); //
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}

