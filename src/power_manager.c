#include "power_manager.h"
#include "app_state.h"
#include "backlight.h"
#include "imu.h"
#include "display_manager.h"
#include <zephyr/kernel.h>

void power_manager_init(void) {
    // Stub
}

void power_manager_panel_sleep(bool on) {
    struct app_runtime *rt = app_state_get();
    if (on) {
        backlight_set_duty(0);
        rt->power.pending_panel_off = true;
    } else {
        display_manager_reinitialize_after_wake();
        backlight_set_duty(255);
        rt->power.display_on = true;
    }
}

void power_manager_sleep_until_tilt(void) {
    // Placeholder for true sleep logic
    k_msleep(100);
}

void power_manager_service_tilt_irq(void) {
    struct app_runtime *rt = app_state_get();
    if (!rt->power.tilt_irq_flag) {
        return;
    }
    rt->power.tilt_irq_flag = false;
    (void)imu_read8(REG_TILT_SRC);
    (void)imu_read8(REG_FUNC_SRC);
    
    if (rt->power.display_on) {
        rt->power.display_expire_ms = k_uptime_get_32() + 10000;
    }
}
