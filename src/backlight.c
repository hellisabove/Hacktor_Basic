#include "backlight.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>

#define BACKLIGHT_NODE DT_NODELABEL(lcd_backlight)
static const struct gpio_dt_spec lcd_bl = GPIO_DT_SPEC_GET(BACKLIGHT_NODE, gpios);

void backlight_init(void) {
    if (gpio_is_ready_dt(&lcd_bl)) {
        gpio_pin_configure_dt(&lcd_bl, GPIO_OUTPUT_ACTIVE);
    }
}

void backlight_set_duty(uint8_t duty) {
    if (gpio_is_ready_dt(&lcd_bl)) {
        gpio_pin_set_dt(&lcd_bl, duty > 0 ? 1 : 0);
    }
}
