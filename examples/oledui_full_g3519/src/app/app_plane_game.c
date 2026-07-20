/**
 * app_plane_game.c
 * 飞机大战游戏主实现文件
 */

#include "app_plane_game.h"
#include "app_game_common.h"
#include "game_port.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_delay.h"
#include "stdlib.h"

#define PLANE_COMBO_TIMEOUT  66
#define PLANE_DYING_FRAMES   15

static int game_status = PLANE_GAME_READY;
static plane_game_object_t player;
static plane_game_object_t enemies[MAX_ENEMIES];
static plane_game_object_t bullets[MAX_BULLETS];
static unsigned int score = 0;
static unsigned int score_counter = 0;
static unsigned int fire_cooldown = 0;
static bool exit_requested = false;

static plane_game_input_t game_input = {
    .up = false, .down = false, .left = false, .right = false, .click = false
};

// 场景与计分
static game_scene_t plane_scene = SCENE_TITLE;
static game_score_t plane_score;
static game_combo_display_t plane_combo_display = {0, 0, false};
static uint16_t scene_frame = 0;
static uint16_t dying_timer = 0;
static uint16_t combo_timeout = 0;
static uint16_t title_anim_frame = 0;

// 前向声明
static void draw_player(void);
static void draw_enemy(plane_game_object_t *enemy);
static void draw_bullet(plane_game_object_t *bullet);
static void plane_title_tick(void);
static void plane_dying_tick(void);
static void plane_result_tick(void);

static void draw_player(void) {
    if (!player.active) return;
    int x = player.x;
    int y = player.y;

    OLED_DrawRectangle(x + 3, y + 3, 7, 4, OLED_FILLED);
    OLED_DrawTriangle(x + 10, y + 3, x + 10, y + 6, x + 13, y + 5, OLED_FILLED);
    OLED_DrawTriangle(x + 4, y + 3, x + 7, y + 3, x + 4, y, OLED_FILLED);
    OLED_DrawLine(x + 4, y, x + 7, y + 2);
    OLED_DrawTriangle(x + 4, y + 6, x + 7, y + 6, x + 4, y + 9, OLED_FILLED);
    OLED_DrawLine(x + 4, y + 9, x + 7, y + 7);
    OLED_DrawLine(x + 2, y + 1, x + 3, y + 3);
    OLED_DrawLine(x + 2, y + 8, x + 3, y + 6);
    OLED_DrawPoint(x + 2, y + 1);
    OLED_DrawPoint(x + 2, y + 8);
    OLED_ClearArea(x + 8, y + 4, 2, 2);
    OLED_DrawPoint(x + 9, y + 4);
}

static void draw_enemy(plane_game_object_t *enemy) {
    if (!enemy->active) return;
    int x = enemy->x;
    int y = enemy->y;

    OLED_DrawRectangle(x + 2, y + 2, 6, 4, OLED_FILLED);
    OLED_DrawTriangle(x + 2, y + 2, x + 2, y + 5, x, y + 4, OLED_FILLED);
    OLED_DrawLine(x + 5, y + 2, x + 7, y);
    OLED_DrawLine(x + 7, y, x + 8, y);
    OLED_DrawLine(x + 8, y, x + 6, y + 2);
    OLED_DrawLine(x + 5, y + 5, x + 7, y + 7);
    OLED_DrawLine(x + 7, y + 7, x + 8, y + 7);
    OLED_DrawLine(x + 8, y + 7, x + 6, y + 5);
    OLED_DrawPoint(x + 8, y + 3);
    OLED_DrawPoint(x + 8, y + 4);
}

static void draw_bullet(plane_game_object_t *bullet) {
    if (!bullet->active) return;
    int x = bullet->x;
    int y = bullet->y;

    OLED_DrawLine(x, y + 1, x + 3, y + 1);
    OLED_DrawPoint(x + 1, y);
    OLED_DrawPoint(x + 1, y + 2);
}

void plane_game_set_click(void) {
    game_input.click = true;
}

void plane_game_set_up(void) {
    game_input.up = true;
    if (player.active && player.y > 0) {
        player.y -= player.speed;
        if (player.y < 0) player.y = 0;
    }
}

void plane_game_set_down(void) {
    game_input.down = true;
    if (player.active && player.y < SCREEN_HEIGHT - player.height) {
        player.y += player.speed;
        if (player.y > SCREEN_HEIGHT - player.height) {
            player.y = SCREEN_HEIGHT - player.height;
        }
    }
}

void plane_game_clear_controls(void) {
    game_input.up = false;
    game_input.down = false;
    game_input.left = false;
    game_input.right = false;
    game_input.click = false;
}

void plane_game_init(void) {
    game_status = PLANE_GAME_READY;
    fire_cooldown = 0;
    exit_requested = false;
    plane_game_clear_controls();

    player.x = PLAYER_INIT_X;
    player.y = SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2;
    player.width = PLAYER_WIDTH;
    player.height = PLAYER_HEIGHT;
    player.speed = PLAYER_SPEED;
    player.active = true;

    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;

    game_common_score_reset(&plane_score);
    game_common_load_save(STORAGE_ID_PLANE, &plane_score);
    plane_combo_display.multiplier = 0;
    plane_combo_display.display_timer = 0;
    plane_combo_display.score_flash = false;
    combo_timeout = 0;
    score = 0;
    score_counter = 0;

    plane_scene = SCENE_TITLE;
    scene_frame = 0;
    dying_timer = 0;
    title_anim_frame = 0;
}

void plane_game_handle_input(void) {
    int16_t enc_delta = Encoder_Get();
    if (enc_delta < 0) plane_game_set_up();
    if (enc_delta > 0) plane_game_set_down();

    if (game_status == PLANE_GAME_RUNNING) {
        // 移动和射击在update中处理
    }

    plane_game_clear_controls();
}

void plane_game_update(void) {
    if (plane_scene != SCENE_RUNNING) return;

    score_counter++;
    if (score_counter >= 30) {
        score++;
        score_counter = 0;
    }

    if (fire_cooldown > 0) fire_cooldown--;

    if (fire_cooldown == 0 && player.active) {
        plane_game_fire_bullet();
        fire_cooldown = FIRE_INTERVAL;
    }

    if (rand() % 3 == 0) {
        plane_game_generate_enemy();
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            enemies[i].x -= enemies[i].speed;
            if ((enemies[i].x <= 0) || (enemies[i].x >= 60000)) {
                enemies[i].active = false;
            }
        }
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].speed;
            if (bullets[i].x > SCREEN_WIDTH) {
                bullets[i].active = false;
            }
        }
    }

    // 子弹-敌机碰撞
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemies[j].active) {
                    if (plane_game_check_collision(&bullets[i], &enemies[j])) {
                        bullets[i].active = false;
                        enemies[j].active = false;
                        game_common_combo_hit(&plane_score, &plane_combo_display);
                        game_common_add_score(&plane_score, 10);
                        combo_timeout = 0;
                    }
                }
            }
        }
    }

    // 玩家-敌机碰撞
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            if (plane_game_check_collision(&player, &enemies[i])) {
                plane_scene = SCENE_DYING;
                dying_timer = 0;
                return;
            }
        }
    }

    // combo超时
    combo_timeout++;
    if (combo_timeout >= PLANE_COMBO_TIMEOUT && plane_score.combo_count > 0) {
        game_common_combo_reset(&plane_score);
    }
    game_common_combo_display_tick(&plane_combo_display);
}

void plane_game_render(void) {
    if (plane_scene != SCENE_RUNNING) return;

    OLED_Clear();
    draw_player();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) draw_enemy(&enemies[i]);
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) draw_bullet(&bullets[i]);
    }

    game_common_combo_display_render(&plane_combo_display, plane_score.current_score);
    OLED_Update();
}

bool plane_game_check_collision(plane_game_object_t *obj1, plane_game_object_t *obj2) {
    bool x_overlap = (obj1->x < obj2->x + obj2->width) &&
                   (obj1->x + obj1->width > obj2->x);
    bool y_overlap = (obj1->y < obj2->y + obj2->height) &&
                   (obj1->y + obj1->height > obj2->y);
    return x_overlap && y_overlap;
}

void plane_game_generate_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x = SCREEN_WIDTH;
            enemies[i].y = rand() % (SCREEN_HEIGHT - ENEMY_HEIGHT);
            enemies[i].width = ENEMY_WIDTH;
            enemies[i].height = ENEMY_HEIGHT;
            enemies[i].speed = ENEMY_SPEED_MIN + (rand() % (ENEMY_SPEED_MAX - ENEMY_SPEED_MIN + 1));
            enemies[i].active = true;
            break;
        }
    }
}

void plane_game_fire_bullet(void) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x + player.width;
            bullets[i].y = player.y + player.height / 2 - BULLET_HEIGHT / 2;
            bullets[i].width = BULLET_WIDTH;
            bullets[i].height = BULLET_HEIGHT;
            bullets[i].speed = BULLET_SPEED;
            bullets[i].active = true;
            break;
        }
    }
}

void plane_game_over(void) {
    plane_scene = SCENE_DYING;
    dying_timer = 0;
}

// SCENE_TITLE
static void plane_title_tick(void) {
    scene_frame++;
    title_anim_frame++;

    int16_t enc = port_encoder_get();
    if (enc != 0 || game_input.click) {
        game_input.click = false;
        plane_scene = SCENE_RUNNING;
        scene_frame = 0;
        game_status = PLANE_GAME_RUNNING;
        return;
    }

    port_clear_screen();

    // 飞机悬浮动画
    int8_t y_offset = 0;
    uint8_t phase = title_anim_frame % 32;
    if (phase < 8) y_offset = (int8_t)(phase / 2);
    else if (phase < 24) y_offset = (int8_t)(4 - (phase - 8) / 2);
    else y_offset = (int8_t)(-4 + (phase - 24) / 2);

    player.y = SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2 + y_offset;
    draw_player();

    game_common_title_render("PLANE WAR", 10, plane_score.high_score, scene_frame);
    port_draw_string(35, 44, "UD to move", FONT_SMALL);
    port_update_screen();
}

// SCENE_DYING
static void plane_dying_tick(void) {
    dying_timer++;
    if (dying_timer >= PLANE_DYING_FRAMES) {
        game_common_save_if_needed(STORAGE_ID_PLANE, &plane_score);
        plane_scene = SCENE_RESULT;
        scene_frame = 0;
        return;
    }

    port_clear_screen();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) draw_enemy(&enemies[i]);
    }
    if (dying_timer % 2 == 0) {
        draw_player();
    }
    port_draw_num(98, 2, plane_score.current_score, 4, FONT_SMALL);
    port_update_screen();
}

// SCENE_RESULT
static void plane_result_tick(void) {
    scene_frame++;

    if (scene_frame > 40 && (port_encoder_get() != 0 || game_input.click)) {
        game_input.click = false;
        plane_game_init();
        return;
    }

    game_common_result_render(&plane_score, scene_frame);
}

void plane_game_loop(void) {
    plane_game_init();

    while (1) {
        if (exit_requested) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        switch (plane_scene) {
            case SCENE_TITLE:
                plane_title_tick();
                break;
            case SCENE_RUNNING:
                plane_game_handle_input();
                plane_game_update();
                plane_game_render();
                plane_game_clear_controls();
                break;
            case SCENE_DYING:
                plane_dying_tick();
                break;
            case SCENE_RESULT:
                plane_result_tick();
                break;
        }

        if (plane_game_should_exit()) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        port_delay_ms(30);
    }
}

void plane_game_request_exit(void) {
    exit_requested = true;
}

bool plane_game_should_exit(void) {
    return exit_requested;
}

void plane_game_tick(void) {
    switch (plane_scene) {
        case SCENE_TITLE:
            plane_title_tick();
            break;
        case SCENE_RUNNING:
            plane_game_handle_input();
            plane_game_update();
            plane_game_render();
            plane_game_clear_controls();
            break;
        case SCENE_DYING:
            plane_dying_tick();
            break;
        case SCENE_RESULT:
            plane_result_tick();
            break;
    }
}

void plane_game_on_exit(void) {
    port_clear_screen();
    port_update_screen();
}
