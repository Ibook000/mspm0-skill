#ifndef _APP_GAME_COMMON_H_
#define _APP_GAME_COMMON_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SCENE_TITLE,
    SCENE_RUNNING,
    SCENE_DYING,
    SCENE_RESULT
} game_scene_t;

#define STORAGE_ID_DINO   0x0001
#define STORAGE_ID_PLANE  0x0002
#define STORAGE_ID_BIRD   0x0003
#define STORAGE_ID_BRICK  0x0004
#define STORAGE_ID_SNAKE  0x0005

#define GAME_SAVE_MAGIC   0xA5A5A5A5

typedef struct {
    uint32_t high_score;
    uint32_t max_combo;
    uint32_t magic;
} game_save_data_t;

typedef struct {
    uint32_t current_score;
    uint32_t high_score;
    uint16_t combo_count;
    uint16_t max_combo;
    uint32_t combo_score;
    bool is_new_record;
    bool is_new_combo_record;
} game_score_t;

typedef struct {
    uint8_t multiplier;
    uint8_t display_timer;
    bool score_flash;
} game_combo_display_t;

void game_common_title_render(const char* title, uint8_t title_y, uint32_t high_score, uint16_t frame);
bool game_common_result_render(game_score_t* score, uint16_t frame);

void game_common_combo_hit(game_score_t* score, game_combo_display_t* display);
void game_common_combo_reset(game_score_t* score);
uint8_t game_common_get_combo_multiplier(uint16_t combo_count);
void game_common_add_score(game_score_t* score, uint32_t base_points);

void game_common_combo_display_tick(game_combo_display_t* display);
void game_common_combo_display_render(game_combo_display_t* display, uint32_t score_value);

void game_common_load_save(uint16_t storage_id, game_score_t* score);
void game_common_save_if_needed(uint16_t storage_id, game_score_t* score);

void game_common_score_reset(game_score_t* score);

#endif // _APP_GAME_COMMON_H_
