#idf_component_register(SRCS "./light_driver.c" "./iot_led.c"
#                    INCLUDE_DIRS "." "./include"
#                    REQUIRES app_storage
#                    REQUIRES driver esp_timer
#)
idf_component_register(
  SRCS "light_driver.c" "iot_led.c"
  INCLUDE_DIRS "include"
  REQUIRES driver esp_timer       # public deps
  PRIV_REQUIRES app_storage       # chỉ nội bộ component dùng
)

