#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
    SCREEN_WATCHFACE,
    SCREEN_INFO
} screen_t;

struct display_state {
    struct tm current_time;
    uint32_t last_tick_ms;
    uint32_t rtc_base_ms;
    screen_t active_screen;
    bool full_refresh;
    uint32_t info_shown_version;
    int info_last_drawn_second;
    int prev_hour_x, prev_hour_y;
    int prev_min_x, prev_min_y;
    int prev_sec_x, prev_sec_y;
    int prev_sec_tail_x, prev_sec_tail_y;
};

struct power_state {
    bool display_on;
    uint32_t display_expire_ms;
    bool pending_sleep;
    bool pending_panel_off;
    volatile bool tilt_irq_flag;
};

struct battery_state {
    uint8_t percent;
    float voltage;
    uint32_t last_poll_ms;
};

struct app_runtime {
    struct display_state display;
    struct power_state power;
    struct battery_state battery;
};

struct app_runtime *app_state_get(void);

#endif /* APP_STATE_H_ */
