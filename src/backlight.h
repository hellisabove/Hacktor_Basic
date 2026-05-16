#ifndef BACKLIGHT_H_
#define BACKLIGHT_H_

#include <stdint.h>

void backlight_init(void);
void backlight_set_duty(uint8_t duty);

#endif /* BACKLIGHT_H_ */
