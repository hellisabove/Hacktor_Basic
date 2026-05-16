#ifndef POWER_MANAGER_H_
#define POWER_MANAGER_H_

#include <stdbool.h>

void power_manager_init(void);
void power_manager_panel_sleep(bool on);
void power_manager_sleep_until_tilt(void);
void power_manager_service_tilt_irq(void);

#endif /* POWER_MANAGER_H_ */
