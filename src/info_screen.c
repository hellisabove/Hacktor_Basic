#include "info_screen.h"
#include "graphics.h"
#include "steps.h"
#include <stdio.h>
#include <string.h>

void info_screen_draw(const struct device *display, struct app_runtime *rt) {
    if (rt->display.full_refresh) {
        graphics_fill_screen(display, 0x0000); // COLOR_BG
        graphics_draw_text(display, 80, 20, "INFO", 0xFFFF, 0x0000, 3);
        
        // Draw static labels
        graphics_draw_text(display, 40, 70,  "Batt: ", 0xFFFF, 0x0000, 2);
        graphics_draw_text(display, 40, 100, "V: ",    0xFFFF, 0x0000, 2);
        graphics_draw_text(display, 40, 130, "Steps: ", 0xFFFF, 0x0000, 2);
        graphics_draw_text(display, 40, 160, "Time: ",  0xFFFF, 0x0000, 2);
        
        rt->display.full_refresh = false;
    }

    char val_buf[24];

    // Update battery percent (pad to 4 chars "100%")
    sprintf(val_buf, "%3d%%", rt->battery.percent);
    graphics_draw_text(display, 112, 70, val_buf, 0xFFFF, 0x0000, 2);

    // Update voltage (pad to 6 chars "4.20V ")
    sprintf(val_buf, "%4.2fV", (double)rt->battery.voltage);
    graphics_draw_text(display, 76, 100, val_buf, 0xFFFF, 0x0000, 2);

    // Update steps (pad to 6 chars)
    sprintf(val_buf, "%6u", steps_get_today());
    graphics_draw_text(display, 124, 130, val_buf, 0xFFFF, 0x0000, 2);

    // Update time (fixed length HH:MM:SS)
    sprintf(val_buf, "%02d:%02d:%02d", 
            rt->display.current_time.tm_hour, 
            rt->display.current_time.tm_min, 
            rt->display.current_time.tm_sec);
    graphics_draw_text(display, 112, 160, val_buf, 0xFFFF, 0x0000, 2);
}
