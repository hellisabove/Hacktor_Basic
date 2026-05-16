#ifndef STEPS_H_
#define STEPS_H_

#include <stdint.h>

void steps_init(uint16_t initial);
void steps_poll(void);
uint32_t steps_get_today(void);

#endif /* STEPS_H_ */
