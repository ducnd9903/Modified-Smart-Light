# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
add_compile_options(-fdiagnostics-color=always)

# Determine whether to define RAIMAKER_PATH
if(NOT DEFINED ENV{RAIMAKER_PATH})
    message(FATAL_ERROR "Please set RAIMAKER_PATH to esp-rainmaker repo")
endif(NOT DEFINED ENV{RAIMAKER_PATH})

# Add RainMaker components and other common application components
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components/light_driver
                        ${CMAKE_CURRENT_LIST_DIR}/../components/button
                        ${CMAKE_CURRENT_LIST_DIR}/../components/app_storage
                        ${CMAKE_CURRENT_LIST_DIR}/../components/app_wifi
                        /home/duc/esp/esp-rainmaker/components/esp-insights/components/
                        # $ENV{RAIMAKER_PATH}/components/esp_rainmaker
                        # $ENV{RAIMAKER_PATH}/components/esp_schedule
                        # $ENV{RAIMAKER_PATH}/components/json_generator
                        # $ENV{RAIMAKER_PATH}/components/json_parser
                        # $ENV{RAIMAKER_PATH}/components/qrcode
                        # $ENV{RAIMAKER_PATH}/components/rmaker_common
                        /home/duc/esp/esp-rainmaker/examples/common/app_insights/
                        )



include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXCLUDE_COMPONENTS "wifi_provisioning" CACHE STRING "")

project(7_insights)
