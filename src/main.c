#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <time.h>

#include "app_state.h"
#include "display_manager.h"
#include "graphics.h"
#include "watchface.h"
#include "backlight.h"
#include "battery_monitor.h"
#include "imu.h"
#include "steps.h"
#include "info_screen.h"
#include "time_keeper.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hacktor, LOG_LEVEL_INF);

#define BTN0_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec btn0 = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);

static struct app_runtime global_runtime;

struct app_runtime *app_state_get(void) {
    return &global_runtime;
}

static void handle_info_button(const struct device *display) {
    static bool last_pressed = false;
    bool pressed = false;
    if (gpio_is_ready_dt(&btn0)) {
        pressed = gpio_pin_get_dt(&btn0) > 0;
    }

    if (pressed && !last_pressed) {
        struct app_runtime *rt = app_state_get();
        rt->power.display_on = true;
        rt->power.display_expire_ms = k_uptime_get_32() + 10000;
        rt->display.full_refresh = true;

        if (rt->display.active_screen == SCREEN_WATCHFACE) {
            rt->display.active_screen = SCREEN_INFO;
        } else {
            rt->display.active_screen = SCREEN_WATCHFACE;
            watchface_draw_full(display, rt);
        }
    }
    last_pressed = pressed;
}

static void refresh_display_if_needed(const struct device *display) {
    struct app_runtime *rt = app_state_get();
    if (!rt->power.display_on) return;

    uint32_t now = k_uptime_get_32();
    uint32_t elapsed_s = (now - rt->display.last_tick_ms) / 1000;
    if (elapsed_s == 0) return;

    time_keeper_apply_elapsed_walltime(elapsed_s);
    rt->display.last_tick_ms += elapsed_s * 1000;

    if (rt->display.active_screen == SCREEN_WATCHFACE) {
        watchface_draw_full(display, rt);
    } else if (rt->display.active_screen == SCREEN_INFO) {
        info_screen_draw(display, rt);
    }
}

int main(void) {
    if (gpio_is_ready_dt(&btn0)) {
        gpio_pin_configure_dt(&btn0, GPIO_INPUT | GPIO_PULL_UP);
    }

    display_manager_init();
    const struct device *display = display_manager_get_device();
    display_manager_reinitialize_after_wake();

    backlight_init();
    watchface_init();
    battery_monitor_init();
    imu_init();
    uint16_t initial_steps = 0;
    if (imu_wait_who_am_i(400)) {
        imu_soft_reset();
        imu_enable_hardware_pedometer(&initial_steps);
        imu_enable_tilt_on_int2();
    }
    steps_init(initial_steps);
    time_keeper_init();

    struct app_runtime *rt = app_state_get();
    rt->display.last_tick_ms = k_uptime_get_32();
    rt->display.active_screen = SCREEN_WATCHFACE;
    rt->power.display_on = true;
    rt->power.display_expire_ms = rt->display.last_tick_ms + 10000;

    graphics_fill_screen(display, 0x0000); // COLOR_BG
    watchface_draw_full(display, rt);
    backlight_set_duty(255);

    while (1) {
        handle_info_button(display);
        refresh_display_if_needed(display);
        battery_monitor_poll();
        steps_poll();
        
        if (rt->power.display_on && k_uptime_get_32() > rt->power.display_expire_ms) {
            rt->power.display_on = false;
            backlight_set_duty(0);
        }

        k_msleep(50);
    }
    return 0;
}
