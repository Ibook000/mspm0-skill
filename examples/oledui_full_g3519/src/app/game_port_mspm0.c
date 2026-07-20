#include "game_port.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_w25qxx.h"
#include "hw_delay.h"

#define GAME_SAVE_SECTOR_ADDR  0xFFE000
#define GAME_SAVE_RECORD_SIZE  16

void port_clear_screen(void) {
    OLED_Clear();
}

void port_update_screen(void) {
    OLED_Update();
}

void port_draw_string(uint8_t x, uint8_t y, const char* str, game_font_t font) {
    uint8_t fs;
    switch (font) {
        case FONT_LARGE:  fs = OLED_10X20_HALF; break;
        case FONT_MEDIUM: fs = OLED_7X12_HALF;  break;
        default:          fs = OLED_6X8_HALF;   break;
    }
    OLED_ShowString(x, y, (char*)str, fs);
}

void port_draw_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, game_font_t font) {
    uint8_t fs;
    switch (font) {
        case FONT_LARGE:  fs = OLED_10X20_HALF; break;
        case FONT_MEDIUM: fs = OLED_7X12_HALF;  break;
        default:          fs = OLED_6X8_HALF;   break;
    }
    OLED_ShowNum(x, y, num, len, fs);
}

void port_draw_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, game_draw_mode_t mode) {
    OLED_DrawRectangle(x, y, w, h, (uint8_t)mode);
}

void port_draw_rounded_rectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, game_draw_mode_t mode) {
    OLED_DrawRoundedRectangle(x, y, w, h, r, (uint8_t)mode);
}

void port_clear_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    OLED_ClearArea(x, y, w, h);
}

void port_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    OLED_DrawLine(x1, y1, x2, y2);
}

void port_draw_point(uint8_t x, uint8_t y) {
    OLED_DrawPoint(x, y);
}

void port_draw_ellipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, game_draw_mode_t mode) {
    OLED_DrawEllipse(x, y, a, b, (uint8_t)mode);
}

void port_draw_triangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, game_draw_mode_t mode) {
    OLED_DrawTriangle(x0, y0, x1, y1, x2, y2, (uint8_t)mode);
}

bool port_storage_read(uint16_t id, void* buf, uint16_t size) {
    if (size > GAME_SAVE_RECORD_SIZE) return false;
    uint32_t addr = GAME_SAVE_SECTOR_ADDR + (uint32_t)id * GAME_SAVE_RECORD_SIZE;
    W25Q128_read((uint8_t*)buf, addr, size);
    return true;
}

bool port_storage_write(uint16_t id, const void* buf, uint16_t size) {
    if (size > GAME_SAVE_RECORD_SIZE) return false;
    static uint8_t sector_buf[64];
    W25Q128_read(sector_buf, GAME_SAVE_SECTOR_ADDR, 64);
    uint16_t offset = (uint16_t)(id * GAME_SAVE_RECORD_SIZE);
    for (uint16_t i = 0; i < size; i++) {
        sector_buf[offset + i] = ((const uint8_t*)buf)[i];
    }
    W25Q128_erase_sector(GAME_SAVE_SECTOR_ADDR / 4096);
    W25Q128_write_page(sector_buf, GAME_SAVE_SECTOR_ADDR, 64);
    return true;
}

int16_t port_encoder_get(void) {
    return Encoder_Get();
}

void port_delay_ms(uint32_t ms) {
    delay_ms(ms);
}
