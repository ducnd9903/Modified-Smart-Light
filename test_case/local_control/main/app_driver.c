/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "iot_button.h"
#include "light_driver.h"

#include DEVELOPMENT_BOARD
#include "app_priv.h"

#define TAG "app_driver"

static bool g_output_state = true;

#define DIMMING_STEP 1
#define DIMMING_DELAY_MS 30
static int g_color = 0;      // 0: đỏ, 1: xanh lá, 2: xanh dương


// static void push_btn_cb(void *arg)
// {
//     app_driver_set_state(!g_output_state);
// }

static void push_btn_cb(void *arg)
{
    // Chuyển sang màu tiếp theo
    g_color = (g_color + 1) % 3;

    // Tắt tất cả màu trước
    light_driver_set_rgb(0, 0, 0);

    // Sáng dần
    for (int brightness = 0; brightness <= 100; brightness += DIMMING_STEP) {
        uint8_t red   = (g_color == 0) ? brightness : 0;
        uint8_t green = (g_color == 1) ? brightness : 0;
        uint8_t blue  = (g_color == 2) ? brightness : 0;
        light_driver_set_rgb(red, green, blue);
        vTaskDelay(pdMS_TO_TICKS(DIMMING_DELAY_MS));
    }
    // Tối dần
    for (int brightness = 100; brightness >= 0; brightness -= DIMMING_STEP) {
        uint8_t red   = (g_color == 0) ? brightness : 0;
        uint8_t green = (g_color == 1) ? brightness : 0;
        uint8_t blue  = (g_color == 2) ? brightness : 0;
        light_driver_set_rgb(red, green, blue);
        vTaskDelay(pdMS_TO_TICKS(DIMMING_DELAY_MS));
    }
    // Tắt hẳn
    light_driver_set_rgb(0, 0, 0);

    ESP_LOGI(TAG, "LED color %d dimming done", g_color);
}

void app_driver_init()
{
    /* Configure push button */
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num     = LIGHT_BUTTON_GPIO,
            .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
        },
    };
    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (btn_handle) {
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, push_btn_cb);
    }

    /**
     * @brief Light driver initialization
     */
    light_driver_config_t driver_config = {
        .gpio_red        = LIGHT_GPIO_RED,
        .gpio_green      = LIGHT_GPIO_GREEN,
        .gpio_blue       = LIGHT_GPIO_BLUE,
        .gpio_cold       = LIGHT_GPIO_COLD,
        .gpio_warm       = LIGHT_GPIO_WARM,
        .fade_period_ms  = LIGHT_FADE_PERIOD_MS,
        .blink_period_ms = LIGHT_BLINK_PERIOD_MS,
        .freq_hz         = LIGHT_FREQ_HZ,
        .clk_cfg         = LEDC_USE_APB_CLK,
        .duty_resolution = LEDC_TIMER_11_BIT,
    };
    ESP_ERROR_CHECK(light_driver_init(&driver_config));
    light_driver_set_switch(true);
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if (g_output_state != state) {
        g_output_state = state;
        if (g_output_state) {
            // light on
            ESP_LOGI(TAG, "Light ON");
            light_driver_set_switch(true);
        } else {
            // light off
            ESP_LOGI(TAG, "Light OFF");
            light_driver_set_switch(false);
        }
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
