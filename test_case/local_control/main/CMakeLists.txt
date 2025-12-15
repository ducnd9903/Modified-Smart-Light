set(srcs
                    "app_driver.c")
set(include_dirs "include")
set(DEVELOPMENT_BOARD "board_esp32c3_devkitc.h")
set(COMPONENT_EMBED_TXTFILES "cacert.pem" "prvtkey.pem")

idf_component_register(SRCS "app_main.c" "app_driver.c"
                       INCLUDE_DIRS "include"
                       EMBED_TXTFILES "cacert.pem" "prvtkey.pem"
                       PRIV_REQUIRES mdns esp_wifi button light_driver app_storage esp_local_ctrl esp_https_server)

idf_build_set_property(COMPILE_DEFINITIONS "-DDEVELOPMENT_BOARD=\"${DEVELOPMENT_BOARD}\"" APPEND)
