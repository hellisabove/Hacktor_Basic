#include "watchface.h"
#include "graphics.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define CENTER_X 120
#define CENTER_Y 119
#define RADIUS   120
#define PI 3.14159265f

static float sin60[60];
static float cos60[60];

void watchface_init(void) {
    for (int i = 0; i < 60; i++) {
        float angle = (float)i * 6.0f * (PI / 180.0f);
        sin60[i] = sinf(angle);
        cos60[i] = cosf(angle);
    }
}

static void draw_ticks(const struct device *display) {
    for (int i = 0; i < 60; i++) {
        float s = sin60[i];
        float c = cos60[i];
        if (i % 5 == 0) {
            int x1 = CENTER_X + (int)(c * (RADIUS - 14));
            int y1 = CENTER_Y + (int)(s * (RADIUS - 14));
            int x2 = CENTER_X + (int)(c * (RADIUS - 2));
            int y2 = CENTER_Y + (int)(s * (RADIUS - 2));
            graphics_draw_line(display, x1, y1, x2, y2, COLOR_FACE);
        } else {
            int x1 = CENTER_X + (int)(c * (RADIUS - 6));
            int y1 = CENTER_Y + (int)(s * (RADIUS - 6));
            int x2 = CENTER_X + (int)(c * (RADIUS - 2));
            int y2 = CENTER_Y + (int)(s * (RADIUS - 2));
            graphics_draw_line(display, x1, y1, x2, y2, COLOR_FACE);
        }
    }
}

void watchface_draw_full(const struct device *display, struct app_runtime *rt) {
    // To avoid flicker, we only erase old hands and draw new ones
    // But if we are called for a full refresh (e.g. switching from INFO), we clear
    static bool first_run = true;
    
    if (first_run || rt->display.full_refresh) {
        graphics_fill_screen(display, COLOR_BG);
        draw_ticks(display);
        // Draw date once
        char date_str[16];
        sprintf(date_str, "%02d/%02d", rt->display.current_time.tm_mday, rt->display.current_time.tm_mon + 1);
        graphics_draw_text(display, 160, 110, date_str, COLOR_FACE, COLOR_BG, 2);
        first_run = false;
        rt->display.full_refresh = false;
        
        // Reset prev positions to force draw
        rt->display.prev_hour_x = rt->display.prev_hour_y = -1;
        rt->display.prev_min_x = rt->display.prev_min_y = -1;
        rt->display.prev_sec_x = rt->display.prev_sec_y = -1;
    }

    // Erase old hands
    if (rt->display.prev_hour_x != -1) {
        graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_hour_x, rt->display.prev_hour_y, COLOR_BG);
    }
    if (rt->display.prev_min_x != -1) {
        graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_min_x, rt->display.prev_min_y, COLOR_BG);
    }
    if (rt->display.prev_sec_x != -1) {
        graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_sec_x, rt->display.prev_sec_y, COLOR_BG);
    }

    // Calc new positions
    int hour = rt->display.current_time.tm_hour % 12;
    int min = rt->display.current_time.tm_min;
    int sec = rt->display.current_time.tm_sec;

    float h_angle = (float)hour * 30.0f + (float)min * 0.5f;
    float m_angle = (float)min * 6.0f;
    float s_angle = (float)sec * 6.0f;

    h_angle = (h_angle - 90.0f) * (PI / 180.0f);
    m_angle = (m_angle - 90.0f) * (PI / 180.0f);
    s_angle = (s_angle - 90.0f) * (PI / 180.0f);

    rt->display.prev_hour_x = CENTER_X + (int)(cosf(h_angle) * 60);
    rt->display.prev_hour_y = CENTER_Y + (int)(sinf(h_angle) * 60);
    rt->display.prev_min_x = CENTER_X + (int)(cosf(m_angle) * 80);
    rt->display.prev_min_y = CENTER_Y + (int)(sinf(m_angle) * 80);
    rt->display.prev_sec_x = CENTER_X + (int)(cosf(s_angle) * 95);
    rt->display.prev_sec_y = CENTER_Y + (int)(sinf(s_angle) * 95);

    // Draw new hands
    graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_hour_x, rt->display.prev_hour_y, COLOR_HOUR_HAND);
    graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_min_x, rt->display.prev_min_y, COLOR_MIN_HAND);
    graphics_draw_line(display, CENTER_X, CENTER_Y, rt->display.prev_sec_x, rt->display.prev_sec_y, COLOR_SEC_HAND);
    
    // Always redraw center dot to clean up hand intersections
    graphics_fill_circle(display, CENTER_X, CENTER_Y, 3, COLOR_FACE);
}
