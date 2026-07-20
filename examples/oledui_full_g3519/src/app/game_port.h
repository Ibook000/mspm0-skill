#ifndef _GAME_PORT_H_
#define _GAME_PORT_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FONT_SMALL,
    FONT_MEDIUM,
    FONT_LARGE
} game_font_t;

typedef enum {
    DRAW_UNFILLED = 0,
    DRAW_FILLED = 1
} game_draw_mode_t;

void port_clear_screen(void);
void port_update_screen(void);
void port_draw_string(uint8_t x, uint8_t y, const char* str, game_font_t font);
void port_draw_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, game_font_t font);
void port_draw_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, game_draw_mode_t mode);
void port_draw_rounded_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, game_draw_mode_t mode);
void port_clear_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void port_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void port_draw_point(uint8_t x, uint8_t y);
void port_draw_ellipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, game_draw_mode_t mode);
void port_draw_triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, game_draw_mode_t mode);

bool port_storage_read(uint16_t id, void* buf, uint16_t size);
bool port_storage_write(uint16_t id, const void* buf, uint16_t size);

int16_t port_encoder_get(void);

void port_delay_ms(uint32_t ms);

#endif // _GAME_PORT_H_
