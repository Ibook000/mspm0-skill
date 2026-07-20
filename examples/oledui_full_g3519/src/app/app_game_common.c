#include "app_game_common.h"
#include "game_port.h"

#define SCREEN_W  128
#define SCREEN_H  64

#define PANEL_X      14
#define PANEL_Y      0
#define PANEL_W      100
#define PANEL_H      64
#define PANEL_R      3
#define SCORE_ANIM_FRAMES  30

void game_common_score_reset(game_score_t* score) {
    score->current_score = 0;
    score->combo_count = 0;
    score->max_combo = 0;
    score->combo_score = 0;
    score->is_new_record = false;
    score->is_new_combo_record = false;
}

uint8_t game_common_get_combo_multiplier(uint16_t combo_count) {
    if (combo_count >= 10) return 30;
    if (combo_count >= 5)  return 20;
    if (combo_count >= 3)  return 15;
    return 10;
}

void game_common_add_score(game_score_t* score, uint32_t base_points) {
    uint8_t mult = game_common_get_combo_multiplier(score->combo_count);
    uint32_t actual = (base_points * mult) / 10;
    if (mult > 10) {
        score->combo_score += actual - base_points;
    }
    score->current_score += actual;
}

void game_common_combo_hit(game_score_t* score, game_combo_display_t* display) {
    score->combo_count++;
    if (score->combo_count > score->max_combo) {
        score->max_combo = score->combo_count;
    }
    uint8_t mult = game_common_get_combo_multiplier(score->combo_count);
    if (mult > 10) {
        display->multiplier = mult;
        display->display_timer = 20;
    }
    if (score->combo_count >= 5) {
        display->score_flash = true;
    }
}

void game_common_combo_reset(game_score_t* score) {
    score->combo_count = 0;
}

void game_common_combo_display_tick(game_combo_display_t* display) {
    if (display->display_timer > 0) {
        display->display_timer--;
    }
    display->score_flash = false;
}

void game_common_combo_display_render(game_combo_display_t* display, uint32_t score_value) {
    if (display->score_flash) {
        port_draw_num(91, 1, score_value, 5, FONT_SMALL);
    } else {
        port_draw_num(93, 2, score_value, 5, FONT_SMALL);
    }
    if (display->display_timer > 0 && display->multiplier > 10) {
        char buf[5];
        if (display->multiplier == 15)      { buf[0]='x'; buf[1]='1'; buf[2]='.'; buf[3]='5'; buf[4]=0; }
        else if (display->multiplier == 20) { buf[0]='x'; buf[1]='2'; buf[2]=0; buf[3]=0; buf[4]=0; }
        else                                { buf[0]='x'; buf[1]='3'; buf[2]=0; buf[3]=0; buf[4]=0; }
        port_draw_string(100, 14, buf, FONT_SMALL);
    }
}

void game_common_load_save(uint16_t storage_id, game_score_t* score) {
    game_save_data_t data;
    port_storage_read(storage_id, &data, sizeof(data));
    if (data.magic == GAME_SAVE_MAGIC) {
        score->high_score = data.high_score;
    } else {
        score->high_score = 0;
    }
}

void game_common_save_if_needed(uint16_t storage_id, game_score_t* score) {
    game_save_data_t data;
    port_storage_read(storage_id, &data, sizeof(data));

    bool need_save = false;
    if (data.magic != GAME_SAVE_MAGIC) {
        data.high_score = score->current_score;
        data.max_combo = score->max_combo;
        data.magic = GAME_SAVE_MAGIC;
        need_save = true;
        score->is_new_record = true;
    } else {
        if (score->current_score > data.high_score) {
            data.high_score = score->current_score;
            need_save = true;
            score->is_new_record = true;
        } else {
            score->is_new_record = false;
        }
        if (score->max_combo > data.max_combo) {
            data.max_combo = score->max_combo;
            need_save = true;
            score->is_new_combo_record = true;
        } else {
            score->is_new_combo_record = false;
        }
    }

    score->high_score = data.high_score;

    if (need_save) {
        port_storage_write(storage_id, &data, sizeof(data));
    }
}

void game_common_title_render(const char* title, uint8_t title_y, uint32_t high_score, uint16_t frame) {
    port_draw_string(30, title_y, title, FONT_SMALL);

    if (high_score > 0) {
        port_draw_string(36, title_y + 13, "HI:", FONT_SMALL);
        port_draw_num(54, title_y + 13, high_score, 5, FONT_SMALL);
    }

    if ((frame / 30) % 2 == 0) {
        port_draw_string(40, title_y + 26, "START", FONT_SMALL);
    }
}

bool game_common_result_render(game_score_t* score, uint16_t frame) {
    port_clear_screen();
    port_draw_rounded_rectangle(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, PANEL_R, DRAW_UNFILLED);

    // 全部使用 FONT_SMALL (实际12px高)，64px屏幕排5行
    // GAME OVER (y=3)
    port_draw_string(34, 3, "GAME OVER", FONT_SMALL);

    // 分数滚动动画
    uint32_t display_score;
    if (frame < SCORE_ANIM_FRAMES) {
        uint32_t step = score->current_score / SCORE_ANIM_FRAMES;
        if (step == 0) step = 1;
        display_score = step * (frame + 1);
        if (display_score > score->current_score) display_score = score->current_score;
    } else {
        display_score = score->current_score;
    }

    // Score行 (y=17)
    port_draw_string(PANEL_X + 8, 17, "Score:", FONT_SMALL);
    port_draw_num(PANEL_X + 50, 17, display_score, 5, FONT_SMALL);

    // Best行 (y=30)
    if (score->is_new_record) {
        if ((frame / 20) % 2 == 0) {
            port_draw_string(PANEL_X + 8, 30, "NEW BEST!", FONT_SMALL);
        }
    } else {
        port_draw_string(PANEL_X + 8, 30, "Best:", FONT_SMALL);
        port_draw_num(PANEL_X + 50, 30, score->high_score, 5, FONT_SMALL);
    }

    // Combo行 (y=43，仅combo>=3时显示)
    if (score->max_combo >= 3) {
        port_draw_string(PANEL_X + 8, 43, "Combo:", FONT_SMALL);
        port_draw_num(PANEL_X + 50, 43, score->max_combo, 3, FONT_SMALL);
    }

    // RETRY (y=52)
    if (frame >= SCORE_ANIM_FRAMES + 10) {
        port_draw_string(33, 52, "- RETRY -", FONT_SMALL);
    }

    port_update_screen();
    return (frame >= SCORE_ANIM_FRAMES);
}
