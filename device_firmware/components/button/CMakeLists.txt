#idf_component_register(SRCS "button_adc.c" "button_gpio.c" "iot_button.c"
#                        INCLUDE_DIRS include
#                        PRIV_REQUIRES esp_adc_cal)
idf_component_register(
  SRCS "iot_button.c" "button_gpio.c" "button_adc_stub.c"
  INCLUDE_DIRS "include"
  REQUIRES driver esp_timer
)

