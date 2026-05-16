#include "steps.h"
#include "imu.h"
#include <zephyr/kernel.h>

static uint32_t steps_today = 0;
static uint16_t last_hw_count = 0;

void steps_init(uint16_t initial) {
    last_hw_count = initial;
    steps_today = 0;
}

void steps_poll(void) {
    uint16_t current;
    imu_read16(REG_STEP_COUNTER_L, &current);
    if (current >= last_hw_count) {
        steps_today += (current - last_hw_count);
    } else {
        steps_today += (0xFFFF - last_hw_count + current + 1);
    }
    last_hw_count = current;
}

uint32_t steps_get_today(void) {
    return steps_today;
}
