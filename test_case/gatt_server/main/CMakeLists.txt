set(srcs "app_main.c"
                    "app_driver.c")
set(include_dirs "include")
set(DEVELOPMENT_BOARD "board_esp32c3_devkitc.h")

idf_component_register(SRCS "app_main.c" "app_driver.c"
                       INCLUDE_DIRS "include"
                       )

idf_build_set_property(COMPILE_DEFINITIONS "-DDEVELOPMENT_BOARD=\"${DEVELOPMENT_BOARD}\"" APPEND)