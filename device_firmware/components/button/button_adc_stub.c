#include "esp_err.h"
#include "button_adc.h"

// Không dùng ADC key trong bài này → stub tối giản cho linker

esp_err_t button_adc_init(const button_adc_config_t *config)
{
    (void)config;
    return ESP_ERR_NOT_SUPPORTED;   // báo không hỗ trợ, nhưng vẫn link được
}

uint8_t button_adc_get_key_level(void *button_index)
{
    (void)button_index;
    return 0;   // coi như không có phím ADC nào được nhấn
}

