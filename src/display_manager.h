#ifndef DISPLAY_MANAGER_H_
#define DISPLAY_MANAGER_H_

#include <zephyr/device.h>

void display_manager_init(void);
const struct device *display_manager_get_device(void);
void display_manager_reinitialize_after_wake(void);

#endif /* DISPLAY_MANAGER_H_ */
