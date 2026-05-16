#ifndef SYSTEM_STATS_H_
#define SYSTEM_STATS_H_

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

struct system_stats {
    uint32_t hard_reset_count;
    uint32_t soft_reset_count;
    uint32_t ble_sync_success;
    uint32_t ble_sync_failures;
    uint32_t screen_turn_ons;
    bool last_ble_sync_valid;
    struct tm last_ble_sync_time;
    uint8_t last_reset_reason;
};

void system_stats_init(void);
void system_stats_record_ble_sync_success(const struct tm *synced_time);
void system_stats_record_ble_sync_failure(void);
void system_stats_record_screen_on_event(void);
const struct system_stats *system_stats_current(void);
uint32_t system_stats_version(void);

#endif /* SYSTEM_STATS_H_ */
