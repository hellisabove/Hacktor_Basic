#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>
#include <zephyr/device.h>

void graphics_fill_screen(const struct device *dev, uint16_t color);
void graphics_fill_rect(const struct device *dev, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void graphics_draw_rect(const struct device *dev, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void graphics_draw_line(const struct device *dev, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void graphics_fill_triangle(const struct device *dev, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void graphics_fill_circle(const struct device *dev, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void graphics_draw_text(const struct device *dev, int16_t x, int16_t y, const char *text, uint16_t color_text, uint16_t color_bg, uint8_t text_size);
void graphics_draw_pixel(const struct device *dev, int16_t x, int16_t y, uint16_t color);

#endif /* GRAPHICS_H_ */
