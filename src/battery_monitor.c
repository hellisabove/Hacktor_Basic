#include "battery_monitor.h"
#include "app_state.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/fuel_gauge.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(hacktor, LOG_LEVEL_INF);

#define FG_NODE DT_NODELABEL(fuel_gauge)
static const struct device *fg_dev = DEVICE_DT_GET(FG_NODE);

void battery_monitor_init(void) {
    if (!device_is_ready(fg_dev)) {
        LOG_ERR("Fuel gauge device not ready");
    }
}

void battery_monitor_poll(void) {
    struct app_runtime *rt = app_state_get();
    if (rt->battery.last_poll_ms != 0 && (k_uptime_get_32() - rt->battery.last_poll_ms < 60000)) return;

    if (!device_is_ready(fg_dev)) return;

    union fuel_gauge_prop_val val;

    if (fuel_gauge_get_prop(fg_dev, FUEL_GAUGE_RELATIVE_STATE_OF_CHARGE, &val) == 0) {
        rt->battery.percent = val.relative_state_of_charge;
    }

    if (fuel_gauge_get_prop(fg_dev, FUEL_GAUGE_VOLTAGE, &val) == 0) {
        // Zephyr voltage is in microvolts (uV)
        rt->battery.voltage = (float)val.voltage / 1000000.0f;
    }

    rt->battery.last_poll_ms = k_uptime_get_32();
}
