cmake_minimum_required(VERSION 3.20.0)

set(BOARD esp32s3_devkitc/esp32s3/procpu CACHE STRING "Default Zephyr board")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hacktor_watch)

target_sources(app PRIVATE
    src/main.c
    src/app_state.h
    src/display_manager.c
    src/graphics.c
    src/watchface.c
    src/backlight.c
    src/battery_monitor.c
    src/imu.c
    src/steps.c
    src/time_keeper.c
    src/info_screen.c
    src/power_manager.c
    src/system_stats.c
)

target_include_directories(app PRIVATE src)
