/**
 * app_dino_game.c
 * 小恐龙游戏主实现文件
 */

#include "app_dino_game.h"
#include "app_game_common.h"
#include "game_port.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "stdlib.h"

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define GROUND_Y        50

#define DINO_WIDTH      14
#define DINO_HEIGHT     14
#define DINO_JUMP_SPEED 4
#define DINO_X          15

#define OBSTACLE_WIDTH   6
#define OBSTACLE_HEIGHT1 12
#define OBSTACLE_HEIGHT2 18
#define OBSTACLE_SPEED   2

#define DINO_COMBO_TIMEOUT  100
#define DINO_DYING_FRAMES   15

// 全局变量
static dino_state_t dino_state = DINO_RUNNING;
static dino_game_object_t dino;
static dino_game_object_t obstacles[3];
static int score = 0;
static int score_counter = 0;
static int speed_level = 1;
static float dino_gravity = 0.3;
static float jump_velocity = 0;
static int animation_frame = 0;
static int ground_offset = 0;
static bool exit_requested = false;

// 场景与计分
static game_scene_t dino_scene = SCENE_TITLE;
static game_score_t dino_score;
static game_combo_display_t dino_combo_display = {0, 0, false};
static uint16_t scene_frame = 0;
static uint16_t dying_timer = 0;
static uint16_t combo_timeout = 0;

// 游戏输入
typedef struct {
    bool click;
} game_input_t;
static game_input_t game_input = {0};

// 前向声明
static void draw_dino(int x, int y, dino_state_t state);
static void draw_obstacle(dino_game_object_t *obs);
static void draw_ground(void);
static void dino_title_tick(void);
static void dino_dying_tick(void);
static void dino_result_tick(void);

// 绘制恐龙
static void draw_dino(int x, int y, dino_state_t state) {
    OLED_ClearArea(x - 2, y - 2, DINO_WIDTH + 4, DINO_HEIGHT + 6);

    if (state == DINO_DEAD) {
        OLED_DrawRectangle(x + 6, y, 8, 6, OLED_FILLED);
        OLED_ClearArea(x + 10, y + 1, 3, 3);
        OLED_DrawLine(x + 10, y + 1, x + 12, y + 3);
        OLED_DrawLine(x + 12, y + 1, x + 10, y + 3);
        OLED_DrawRectangle(x + 10, y + 5, 4, 1, OLED_FILLED);
        OLED_DrawRectangle(x + 4, y + 5, 6, 7, OLED_FILLED);
        OLED_DrawRectangle(x + 9, y + 7, 2, 3, OLED_FILLED);
        OLED_DrawRectangle(x, y + 4, 2, 2, OLED_FILLED);
        OLED_DrawRectangle(x + 2, y + 5, 2, 2, OLED_FILLED);
        OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
        OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
    } else if (state == DINO_DUCKING) {
        int dy = 6;
        OLED_DrawRectangle(x + 10, y + dy, 6, 4, OLED_FILLED);
        OLED_ClearArea(x + 13, y + dy + 1, 2, 2);
        OLED_DrawPoint(x + 14, y + dy + 1);
        OLED_DrawRectangle(x, y + dy + 3, 12, 4, OLED_FILLED);
        OLED_DrawRectangle(x + 11, y + dy + 4, 2, 2, OLED_FILLED);
        if (animation_frame % 8 < 4) {
            OLED_DrawRectangle(x + 3, y + dy + 7, 2, 3, OLED_FILLED);
            OLED_DrawPoint(x + 7, y + dy + 7);
        } else {
            OLED_DrawPoint(x + 3, y + dy + 7);
            OLED_DrawRectangle(x + 7, y + dy + 7, 2, 3, OLED_FILLED);
        }
    } else {
        OLED_DrawRectangle(x + 6, y, 8, 6, OLED_FILLED);
        OLED_ClearArea(x + 10, y + 1, 2, 2);
        OLED_DrawPoint(x + 11, y + 1);
        OLED_DrawRectangle(x + 10, y + 5, 4, 1, OLED_FILLED);
        OLED_DrawRectangle(x + 4, y + 5, 6, 7, OLED_FILLED);
        OLED_DrawRectangle(x + 9, y + 7, 2, 3, OLED_FILLED);
        OLED_DrawRectangle(x, y + 4, 2, 2, OLED_FILLED);
        OLED_DrawRectangle(x + 2, y + 5, 2, 2, OLED_FILLED);

        if (state == DINO_JUMPING) {
            OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
            OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
        } else {
            if (animation_frame % 8 < 4) {
                OLED_DrawRectangle(x + 5, y + 12, 2, 3, OLED_FILLED);
                OLED_DrawPoint(x + 5, y + 14);
                OLED_DrawRectangle(x + 8, y + 12, 2, 2, OLED_FILLED);
            } else {
                OLED_DrawRectangle(x + 5, y + 12, 2, 2, OLED_FILLED);
                OLED_DrawRectangle(x + 8, y + 12, 2, 3, OLED_FILLED);
                OLED_DrawPoint(x + 8, y + 14);
            }
        }
    }
}

// 绘制障碍物
static void draw_obstacle(dino_game_object_t *obs) {
    if (obs->width == 0 || obs->height == 0) return;

    OLED_ClearArea(obs->x - 3, obs->y - 1, obs->width + 6, obs->height + 2);

    if (obs->height == OBSTACLE_HEIGHT1) {
        int bx = obs->x;
        int by = obs->y;
        OLED_DrawRectangle(bx + 2, by, 2, 12, OLED_FILLED);
        OLED_DrawRectangle(bx, by + 3, 2, 4, OLED_FILLED);
        OLED_DrawPoint(bx, by + 3);
        OLED_DrawRectangle(bx + 4, by + 5, 2, 3, OLED_FILLED);
        OLED_DrawPoint(bx + 5, by + 5);
        OLED_DrawPoint(bx + 2, by - 1);
        OLED_DrawPoint(bx + 3, by - 1);
    } else {
        int bx = obs->x;
        int by = obs->y;
        OLED_DrawRectangle(bx, by + 2, 2, 16, OLED_FILLED);
        OLED_DrawRectangle(bx - 2, by + 6, 2, 4, OLED_FILLED);
        OLED_DrawRectangle(bx + 2, by + 9, 1, 3, OLED_FILLED);
        OLED_DrawPoint(bx, by + 1);
        OLED_DrawPoint(bx + 1, by + 1);
        OLED_DrawRectangle(bx + 4, by + 5, 2, 13, OLED_FILLED);
        OLED_DrawRectangle(bx + 6, by + 8, 2, 4, OLED_FILLED);
        OLED_DrawPoint(bx + 4, by + 4);
        OLED_DrawPoint(bx + 5, by + 4);
    }
}

// 绘制地面
static void draw_ground(void) {
    OLED_DrawLine(0, GROUND_Y, SCREEN_WIDTH - 1, GROUND_Y);

    int off = ground_offset % 128;
    for (int i = -off; i < SCREEN_WIDTH; i += 6) {
        int px = i + ((i * 7 + 13) % 5);
        if (px >= 0 && px < SCREEN_WIDTH) {
            OLED_DrawPoint(px, GROUND_Y + 2);
        }
    }
    for (int i = -off; i < SCREEN_WIDTH; i += 11) {
        int px = i + ((i * 3 + 7) % 4);
        if (px >= 0 && px < SCREEN_WIDTH - 1) {
            OLED_DrawLine(px, GROUND_Y + 3, px + 1, GROUND_Y + 3);
        }
    }
    for (int i = -off; i < SCREEN_WIDTH; i += 17) {
        int px = i + ((i * 11 + 3) % 7);
        if (px >= 0 && px < SCREEN_WIDTH) {
            OLED_DrawPoint(px, GROUND_Y + 5);
        }
    }
}

// 初始化游戏
void dino_game_init(void) {
    srand(animation_frame);

    dino.x = DINO_X;
    dino.y = GROUND_Y - DINO_HEIGHT;
    dino.width = DINO_WIDTH;
    dino.height = DINO_HEIGHT;
    dino_state = DINO_RUNNING;
    jump_velocity = 0;

    for (int i = 0; i < 3; i++) {
        obstacles[i].x = 0;
        obstacles[i].y = 0;
        obstacles[i].width = 0;
        obstacles[i].height = 0;
        obstacles[i].speed = OBSTACLE_SPEED;
    }

    game_common_score_reset(&dino_score);
    game_common_load_save(STORAGE_ID_DINO, &dino_score);
    dino_combo_display.multiplier = 0;
    dino_combo_display.display_timer = 0;
    dino_combo_display.score_flash = false;
    combo_timeout = 0;

    dino_scene = SCENE_TITLE;
    scene_frame = 0;
    dying_timer = 0;
    score = 0;
    score_counter = 0;
    speed_level = 1;
    animation_frame = 0;
    ground_offset = 0;
    exit_requested = false;
    game_input.click = 0;
}

// 处理按键输入
void dino_game_handle_input(void) {
    if (Encoder_Get() != 0) {
        game_input.click = 1;
    }

    if (dino_scene == SCENE_RUNNING) {
        if (game_input.click && dino_state != DINO_JUMPING && dino.y >= GROUND_Y - DINO_HEIGHT) {
            game_input.click = 0;
            dino_state = DINO_JUMPING;
            jump_velocity = -DINO_JUMP_SPEED;
        }
    }
    game_input.click = 0;
}

// 更新游戏状态
void dino_game_update(void) {
    if (dino_scene != SCENE_RUNNING) return;

    animation_frame++;

    int ground_speed = 1 + (speed_level - 1) / 3;
    ground_offset += ground_speed;
    if (ground_offset >= SCREEN_WIDTH) {
        ground_offset = 0;
    }

    // 更新分数（基于时间）
    score_counter++;
    if (score_counter >= 10) {
        score_counter = 0;
        score++;
        if (score % 100 == 0 && speed_level < 5) {
            speed_level++;
            for (int i = 0; i < 3; i++) {
                obstacles[i].speed = OBSTACLE_SPEED + (speed_level - 1) / 2;
            }
        }
    }

    // 更新恐龙物理
    if (dino_state == DINO_JUMPING) {
        jump_velocity += dino_gravity;
        dino.y += jump_velocity;
        if (dino.y <= 5) {
            dino.y = 5;
            if (jump_velocity < 0) {
                jump_velocity = 0.1;
            }
        }
        if (dino.y >= GROUND_Y - DINO_HEIGHT) {
            dino.y = GROUND_Y - DINO_HEIGHT;
            dino_state = DINO_RUNNING;
            jump_velocity = 0;
        }
    }

    // 障碍物生成计时器
    static int obstacle_spawn_timer = 0;
    obstacle_spawn_timer++;

    // 更新障碍物位置并检测通过
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].x > 0) {
            unsigned int old_right = obstacles[i].x + obstacles[i].width;
            obstacles[i].x -= obstacles[i].speed;

            // 检测障碍物刚刚通过恐龙
            if (old_right >= dino.x && obstacles[i].x + obstacles[i].width < dino.x) {
                game_common_combo_hit(&dino_score, &dino_combo_display);
                game_common_add_score(&dino_score, 10);
                combo_timeout = 0;
            }

            if (obstacles[i].x <= 0) {
                obstacles[i].x = 0;
                obstacles[i].width = 0;
                obstacles[i].height = 0;
            }
        }
    }

    // combo超时检测
    combo_timeout++;
    if (combo_timeout >= DINO_COMBO_TIMEOUT && dino_score.combo_count > 0) {
        game_common_combo_reset(&dino_score);
    }
    game_common_combo_display_tick(&dino_combo_display);

    // 障碍物生成逻辑：随机间隔 + 安全距离
    uint16_t base_interval = 80 - speed_level * 8;
    uint16_t random_range = 40 + speed_level * 5;
    uint16_t spawn_interval = base_interval + (rand() % random_range);
    if (spawn_interval < 35) spawn_interval = 35;

    int active_obstacles = 0;
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].x > 0) active_obstacles++;
    }

    bool should_spawn = (obstacle_spawn_timer >= spawn_interval && active_obstacles < 3);

    if (should_spawn && active_obstacles > 0) {
        int rightmost_x = -1;
        for (int i = 0; i < 3; i++) {
            if (obstacles[i].x > 0 && (int)obstacles[i].x > rightmost_x) {
                rightmost_x = (int)obstacles[i].x;
            }
        }
        int min_distance = 40 + speed_level * 5;
        should_spawn = (rightmost_x < (int)SCREEN_WIDTH - min_distance);
    }

    if (should_spawn) {
        for (int i = 0; i < 3; i++) {
            if (obstacles[i].width == 0) {
                bool is_big = (rand() % 4 == 0);
                obstacles[i].x = SCREEN_WIDTH + 10;
                obstacles[i].y = GROUND_Y - (is_big ? OBSTACLE_HEIGHT2 : OBSTACLE_HEIGHT1);
                obstacles[i].width = OBSTACLE_WIDTH;
                obstacles[i].height = is_big ? OBSTACLE_HEIGHT2 : OBSTACLE_HEIGHT1;
                obstacles[i].speed = OBSTACLE_SPEED + (speed_level - 1) / 2;
                obstacle_spawn_timer = 0;
                break;
            }
        }
    }

    // 碰撞检测
    if (dino_game_check_collision()) {
        dino_state = DINO_DEAD;
        dino_scene = SCENE_DYING;
        dying_timer = 0;
    }
}

// 渲染游戏画面
void dino_game_render(void) {
    if (dino_scene != SCENE_RUNNING) return;

    OLED_Clear();
    draw_ground();
    draw_dino(dino.x, dino.y, dino_state);

    for (int i = 0; i < 3; i++) {
        if (obstacles[i].width > 0) {
            draw_obstacle(&obstacles[i]);
        }
    }

    game_common_combo_display_render(&dino_combo_display, dino_score.current_score);
    OLED_Update();
}

// 碰撞检测
bool dino_game_check_collision(void) {
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].width > 0) {
            bool x_overlap = (dino.x < obstacles[i].x + obstacles[i].width) &&
                           (dino.x + dino.width > obstacles[i].x);
            bool y_overlap = (dino.y < obstacles[i].y + obstacles[i].height) &&
                           (dino.y + dino.height > obstacles[i].y);
            if (x_overlap && y_overlap) {
                return true;
            }
        }
    }
    return false;
}

void dino_game_generate_obstacle(void) {
}

void dino_game_over(void) {
    dino_state = DINO_DEAD;
    dino_scene = SCENE_DYING;
    dying_timer = 0;
}

void dino_game_request_exit(void) {
    exit_requested = true;
}

bool dino_game_should_exit(void) {
    return exit_requested;
}

// SCENE_TITLE
static void dino_title_tick(void) {
    scene_frame++;
    animation_frame++;

    // 检测输入（编码器或按键）
    if (port_encoder_get() != 0 || game_input.click) {
        game_input.click = 0;
        dino_scene = SCENE_RUNNING;
        scene_frame = 0;
        port_clear_screen();
        return;
    }

    port_clear_screen();
    draw_ground();
    draw_dino(DINO_X, GROUND_Y - DINO_HEIGHT, DINO_RUNNING);
    game_common_title_render("DINO GAME", 12, dino_score.high_score, scene_frame);
    port_update_screen();
}

// SCENE_DYING
static void dino_dying_tick(void) {
    dying_timer++;
    if (dying_timer >= DINO_DYING_FRAMES) {
        game_common_save_if_needed(STORAGE_ID_DINO, &dino_score);
        dino_scene = SCENE_RESULT;
        scene_frame = 0;
        return;
    }
    port_clear_screen();
    draw_ground();
    draw_dino(dino.x, dino.y, DINO_DEAD);
    for (int i = 0; i < 3; i++) {
        if (obstacles[i].width > 0) draw_obstacle(&obstacles[i]);
    }
    port_draw_num(93, 2, dino_score.current_score, 5, FONT_SMALL);
    port_update_screen();
}

// SCENE_RESULT
static void dino_result_tick(void) {
    scene_frame++;

    // 检测输入重新开始（编码器或按键）
    if (scene_frame > 40 && (port_encoder_get() != 0 || game_input.click)) {
        game_input.click = 0;
        dino_game_init();
        return;
    }

    game_common_result_render(&dino_score, scene_frame);
}

void dino_game_tick(void) {
    switch (dino_scene) {
        case SCENE_TITLE:
            dino_title_tick();
            break;
        case SCENE_RUNNING:
            dino_game_handle_input();
            dino_game_update();
            dino_game_render();
            break;
        case SCENE_DYING:
            dino_dying_tick();
            break;
        case SCENE_RESULT:
            dino_result_tick();
            break;
    }
}

void dino_game_on_exit(void) {
    port_clear_screen();
    port_update_screen();
}

void dino_game_set_click(void) {
    game_input.click = 1;
}

void dino_game_loop(void) {
    dino_game_init();

    while (1) {
        if (exit_requested) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        switch (dino_scene) {
            case SCENE_TITLE:
                dino_title_tick();
                break;
            case SCENE_RUNNING:
                dino_game_handle_input();
                dino_game_update();
                dino_game_render();
                break;
            case SCENE_DYING:
                dino_dying_tick();
                break;
            case SCENE_RESULT:
                dino_result_tick();
                break;
        }

        port_delay_ms(30);
    }
}
