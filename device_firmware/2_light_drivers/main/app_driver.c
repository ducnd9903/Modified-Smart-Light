#include <stdio.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"

#include "iot_button.h"
#include "light_driver.h"

#include DEVELOPMENT_BOARD
#include "app_priv.h"

#define TAG "app_driver"
#define DIMMING_STEP 1          // Bước tăng/giảm brightness mỗi lần (1% để mượt)
#define DIMMING_DELAY_MS 50     // Delay giữa các bước (ms) - càng nhỏ càng mượt

static bool g_output_state = false;  // Ban đầu đèn tắt
static bool g_light_active = false;  // Trạng thái hoạt động của đèn
static uint8_t brightness = 0; 
static bool dimming_up = true;
static uint8_t current_color = 0; // 0=Red, 1=Green, 2=Blue
static TaskHandle_t dimming_task_handle = NULL;

// Màu RGB cho từng trạng thái
static const uint8_t colors[3][3] = {
    {100, 0, 0},    // Đỏ
    {0, 100, 0},    // Xanh lá
    {0, 0, 100}     // Xanh dương
};

// Task điều khiển dimming
static void dimming_task(void *pvParameters)
{
    while (1) {
        if (g_light_active) {
            if (dimming_up) {
                brightness += DIMMING_STEP;
                if (brightness >= 100) {
                    brightness = 100;
                    dimming_up = false;
                    ESP_LOGI(TAG, "Reached max brightness, starting to dim down");
                }
            } else {
                if (brightness <= DIMMING_STEP) {
                    brightness = 0;
                    dimming_up = true;
                    // Chuyển sang màu tiếp theo
                    current_color = (current_color + 1) % 3;
                    ESP_LOGI(TAG, "Color changed to: %d", current_color);
                } else {
                    brightness -= DIMMING_STEP;
                }
            }
            
            // Tính toán giá trị RGB dựa trên brightness và màu hiện tại
            uint8_t red = (colors[current_color][0] * brightness) / 100;
            uint8_t green = (colors[current_color][1] * brightness) / 100;
            uint8_t blue = (colors[current_color][2] * brightness) / 100;
            
            // Cập nhật LED
            light_driver_set_rgb(red, green, blue);
        }
        
        vTaskDelay(pdMS_TO_TICKS(DIMMING_DELAY_MS));
    }
}

static void push_btn_cb(void *arg)
{
    if (!g_light_active) {
        // Bật đèn
        g_light_active = true;
        g_output_state = true;
        brightness = 0;
        dimming_up = true;
        current_color = 0; // Bắt đầu với màu đỏ
        
        light_driver_set_switch(true);
        ESP_LOGI(TAG, "Light turned ON - Starting dimming cycle");
        
        // Tạo task dimming nếu chưa có
        if (dimming_task_handle == NULL) {
            xTaskCreate(dimming_task, "dimming_task", 2048, NULL, 5, &dimming_task_handle);
        }
    } else {
        // Tắt đèn
        g_light_active = false;
        g_output_state = false;
        brightness = 0;
        
        light_driver_set_rgb(0, 0, 0);  // Tắt hoàn toàn
        light_driver_set_switch(false);
        ESP_LOGI(TAG, "Light turned OFF");
        
        // Không xóa task, chỉ dừng hoạt động
    }
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
        /* Register a callback for a button short press event */
        iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
        ESP_LOGI(TAG, "Button initialized successfully");
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
    
    // Ban đầu tắt đèn
    light_driver_set_switch(false);
    light_driver_set_rgb(0, 0, 0);
    ESP_LOGI(TAG, "Light driver initialized - Light OFF");
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if (g_output_state != state) {
        g_output_state = state;
        if (state) {
            // Bật đèn thông qua remote hoặc app
            if (!g_light_active) {
                g_light_active = true;
                brightness = 0;
                dimming_up = true;
                current_color = 0;
                
                light_driver_set_switch(true);
                ESP_LOGI(TAG, "Light ON via external control");
                
                if (dimming_task_handle == NULL) {
                    xTaskCreate(dimming_task, "dimming_task", 2048, NULL, 5, &dimming_task_handle);
                }
            }
        } else {
            // Tắt đèn
            g_light_active = false;
            brightness = 0;
            light_driver_set_rgb(0, 0, 0);
            light_driver_set_switch(false);
            ESP_LOGI(TAG, "Light OFF via external control");
        }
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}