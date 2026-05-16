#include "time_keeper.h"
#include "app_state.h"
#include "steps.h"
#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

static int weekday_from_date(int y, int m, int d) {
    if (m < 3) {
        m += 12;
        y--;
    }
    int K = y % 100;
    int J = y / 100;
    int h = (d + 13 * (m + 1) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    return (h + 6) % 7;
}

static struct tm build_compile_time_tm(void) {
    struct tm compile_time = {0};
    const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char m_str[4] = {0};
    int day = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;

    sscanf(__DATE__, "%3s %d %d", m_str, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);
    
    const char *m_ptr = strstr(months, m_str);
    int month = 0;
    if (m_ptr) {
        month = (int)((m_ptr - months) / 3) + 1;
    }

    compile_time.tm_sec = sec;
    compile_time.tm_min = min;
    compile_time.tm_hour = hour;
    compile_time.tm_mday = day;
    compile_time.tm_mon = month - 1;
    compile_time.tm_year = year - 1900;
    compile_time.tm_wday = weekday_from_date(year, month, day);
    compile_time.tm_isdst = -1;

    return compile_time;
}

void time_keeper_init(void) {
    struct app_runtime *rt = app_state_get();
    // For now, no persistence implemented, always start from compile time
    rt->display.current_time = build_compile_time_tm();
}

void time_keeper_apply_elapsed_walltime(uint32_t seconds) {
    struct app_runtime *rt = app_state_get();
    
    // Use standard mktime/localtime to handle rollovers properly
    time_t now = mktime(&rt->display.current_time);
    int old_mday = rt->display.current_time.tm_mday;
    
    now += seconds;
    
    struct tm *new_tm = localtime(&now);
    if (new_tm) {
        rt->display.current_time = *new_tm;
        
        // Check if day changed to reset steps baseline (mock for now)
        if (rt->display.current_time.tm_mday != old_mday) {
            // steps_reset_daily_baseline(); // If we had this function
        }
    }
}
