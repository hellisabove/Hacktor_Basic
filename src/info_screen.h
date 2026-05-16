#ifndef INFO_SCREEN_H_
#define INFO_SCREEN_H_

#include <zephyr/device.h>
#include "app_state.h"

void info_screen_draw(const struct device *display, struct app_runtime *rt);

#endif /* INFO_SCREEN_H_ */
