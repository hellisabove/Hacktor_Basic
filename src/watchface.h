#ifndef WATCHFACE_H_
#define WATCHFACE_H_

#include <zephyr/device.h>
#include "app_state.h"

#define COLOR_BG        0x0000
#define COLOR_FACE      0xFFFF
#define COLOR_HOUR_HAND 0xFFFF
#define COLOR_MIN_HAND  0xFFFF
#define COLOR_SEC_HAND  0xF800
#define COLOR_DATE_NUM  0xF800
#define COLOR_STEPS     0xFFFF

void watchface_init(void);
void watchface_draw_full(const struct device *display, struct app_runtime *rt);

#endif /* WATCHFACE_H_ */
