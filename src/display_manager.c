#include "display_manager.h"
#include <zephyr/drivers/display.h>

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

static const struct device *display_dev;

void display_manager_init(void) {
    display_dev = DEVICE_DT_GET(DISPLAY_NODE);
    if (!device_is_ready(display_dev)) {
        // Handle error
    }
}

const struct device *display_manager_get_device(void) {
    return display_dev;
}

void display_manager_reinitialize_after_wake(void) {
    if (display_dev) {
        display_blanking_off(display_dev);
    }
}
