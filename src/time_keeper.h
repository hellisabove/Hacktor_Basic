#ifndef TIME_KEEPER_H_
#define TIME_KEEPER_H_

#include <stdint.h>

void time_keeper_init(void);
void time_keeper_apply_elapsed_walltime(uint32_t seconds);

#endif /* TIME_KEEPER_H_ */
