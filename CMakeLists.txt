cmake_minimum_required(VERSION 3.22)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Only build these components (and whatever they themselves require)
set(COMPONENTS
    main
    freertos
    esp_timer
    esp_log
    esp_netif
    esp_event
    esp_wifi
    nvs_flash
    driver
)

# Now bring in the normal IDF machinery
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESPC5Marauder)