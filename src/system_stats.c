#include "system_stats.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <string.h>

#define STATS_MAGIC 0x48535453

struct persisted_stats {
    uint32_t magic;
    struct system_stats stats;
};

// Using static for now, as noinit requires specific linker setup
static struct persisted_stats s_persisted;
static uint32_t s_version = 0;

void system_stats_init(void) {
    if (s_persisted.magic != STATS_MAGIC) {
        memset(&s_persisted, 0, sizeof(s_persisted));
        s_persisted.magic = STATS_MAGIC;
    }
    
    // In Zephyr we could use sys_reboot_reason_get() if available
    s_persisted.stats.soft_reset_count++;
    s_version++;
}

void system_stats_record_ble_sync_success(const struct tm *synced_time) {
    s_persisted.stats.ble_sync_success++;
    s_persisted.stats.last_ble_sync_valid = true;
    s_persisted.stats.last_ble_sync_time = *synced_time;
    s_version++;
}

void system_stats_record_ble_sync_failure(void) {
    s_persisted.stats.ble_sync_failures++;
    s_version++;
}

void system_stats_record_screen_on_event(void) {
    s_persisted.stats.screen_turn_ons++;
    s_version++;
}

const struct system_stats *system_stats_current(void) {
    return &s_persisted.stats;
}

uint32_t system_stats_version(void) {
    return s_version;
}
