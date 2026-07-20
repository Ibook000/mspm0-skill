/**
 * app_brick_game.c
 * 打砖块游戏实现
 */

#include "app_brick_game.h"
#include "app_game_common.h"
#include "game_port.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "stdlib.h"

#define BRICK_COMBO_TIMEOUT  60
#define BRICK_DYING_FRAMES   15

// 砖块状态
static bool bricks[BRICK_ROWS][BRICK_COLS];
static int bricks_remaining = 0;

// 挡板
static int paddle_x;

// 球
static int ball_x, ball_y;
static int ball_dx, ball_dy;
static bool ball_launched;

// 场景与计分
static game_scene_t brick_scene = SCENE_TITLE;
static game_score_t brick_score;
static game_combo_display_t brick_combo_display = {0, 0, false};
static uint16_t scene_frame = 0;
static uint16_t dying_timer = 0;
static uint16_t combo_timeout = 0;
static uint16_t animation_frame = 0;

static bool exit_requested = false;
static bool click_input = false;

// 前向声明
static void brick_title_tick(void);
static void brick_dying_tick(void);
static void brick_result_tick(void);
static void brick_running_tick(void);
static void draw_bricks(void);
static void draw_paddle(void);
static void draw_ball(void);
static void reset_level(void);

static void reset_level(void) {
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c] = true;
        }
    }
    bricks_remaining = BRICK_ROWS * BRICK_COLS;

    paddle_x = SCREEN_W / 2 - PADDLE_WIDTH / 2;
    ball_launched = false;
    ball_x = paddle_x + PADDLE_WIDTH / 2;
    ball_y = PADDLE_Y - BALL_SIZE - 1;
    ball_dx = BALL_SPEED_X;
    ball_dy = BALL_SPEED_Y;
}

void brick_game_init(void) {
    game_common_score_reset(&brick_score);
    game_common_load_save(STORAGE_ID_BRICK, &brick_score);
    brick_combo_display.multiplier = 0;
    brick_combo_display.display_timer = 0;
    brick_combo_display.score_flash = false;
    combo_timeout = 0;

    brick_scene = SCENE_TITLE;
    scene_frame = 0;
    dying_timer = 0;
    animation_frame = 0;
    exit_requested = false;
    click_input = false;

    reset_level();
}

void brick_game_set_click(void) {
    click_input = true;
}

void brick_game_request_exit(void) {
    exit_requested = true;
}

bool brick_game_should_exit(void) {
    return exit_requested;
}

static void draw_bricks(void) {
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c]) {
                int x = c * (BRICK_WIDTH + BRICK_GAP) + 4;
                int y = BRICK_TOP_Y + r * (BRICK_HEIGHT + BRICK_GAP);
                OLED_DrawRectangle(x, y, BRICK_WIDTH, BRICK_HEIGHT, OLED_FILLED);
            }
        }
    }
}

static void draw_paddle(void) {
    OLED_DrawRectangle(paddle_x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, OLED_FILLED);
}

static void draw_ball(void) {
    OLED_DrawRectangle(ball_x, ball_y, BALL_SIZE, BALL_SIZE, OLED_FILLED);
}

static void brick_title_tick(void) {
    scene_frame++;
    animation_frame++;

    if (port_encoder_get() != 0 || click_input) {
        click_input = false;
        brick_scene = SCENE_RUNNING;
        scene_frame = 0;
        return;
    }

    port_clear_screen();
    draw_bricks();
    draw_paddle();
    draw_ball();
    game_common_title_render("BREAKOUT", 20, brick_score.high_score, scene_frame);
    port_update_screen();
}

static void brick_running_tick(void) {
    animation_frame++;

    // 输入处理
    int16_t enc = port_encoder_get();
    if (enc != 0) {
        paddle_x += enc * PADDLE_SPEED;
        if (paddle_x < 0) paddle_x = 0;
        if (paddle_x > SCREEN_W - PADDLE_WIDTH) paddle_x = SCREEN_W - PADDLE_WIDTH;
    }

    if (click_input) {
        click_input = false;
        if (!ball_launched) {
            ball_launched = true;
            ball_dx = BALL_SPEED_X;
            ball_dy = BALL_SPEED_Y;
        }
    }

    // 球跟随挡板（未发射时）
    if (!ball_launched) {
        ball_x = paddle_x + PADDLE_WIDTH / 2 - BALL_SIZE / 2;
        ball_y = PADDLE_Y - BALL_SIZE - 1;
    } else {
        // 移动球
        ball_x += ball_dx;
        ball_y += ball_dy;

        // 左右墙壁反弹
        if (ball_x <= 0) {
            ball_x = 0;
            ball_dx = -ball_dx;
        }
        if (ball_x >= SCREEN_W - BALL_SIZE) {
            ball_x = SCREEN_W - BALL_SIZE;
            ball_dx = -ball_dx;
        }
        // 顶部反弹
        if (ball_y <= 0) {
            ball_y = 0;
            ball_dy = -ball_dy;
        }

        // 球落到底部 - 死亡
        if (ball_y >= SCREEN_H) {
            brick_scene = SCENE_DYING;
            dying_timer = 0;
            return;
        }

        // 挡板碰撞
        if (ball_dy > 0 &&
            ball_y + BALL_SIZE >= PADDLE_Y &&
            ball_y + BALL_SIZE <= PADDLE_Y + PADDLE_HEIGHT + 2 &&
            ball_x + BALL_SIZE > paddle_x &&
            ball_x < paddle_x + PADDLE_WIDTH) {
            ball_dy = -ball_dy;
            ball_y = PADDLE_Y - BALL_SIZE;
            // 根据击中挡板位置调整水平方向
            int hit_pos = (ball_x + BALL_SIZE / 2) - (paddle_x + PADDLE_WIDTH / 2);
            if (hit_pos < -6) ball_dx = -3;
            else if (hit_pos < -2) ball_dx = -2;
            else if (hit_pos > 6) ball_dx = 3;
            else if (hit_pos > 2) ball_dx = 2;
        }

        // 砖块碰撞
        for (int r = 0; r < BRICK_ROWS; r++) {
            for (int c = 0; c < BRICK_COLS; c++) {
                if (!bricks[r][c]) continue;
                int bx = c * (BRICK_WIDTH + BRICK_GAP) + 4;
                int by = BRICK_TOP_Y + r * (BRICK_HEIGHT + BRICK_GAP);

                if (ball_x + BALL_SIZE > bx && ball_x < bx + BRICK_WIDTH &&
                    ball_y + BALL_SIZE > by && ball_y < by + BRICK_HEIGHT) {
                    bricks[r][c] = false;
                    bricks_remaining--;
                    ball_dy = -ball_dy;

                    game_common_combo_hit(&brick_score, &brick_combo_display);
                    game_common_add_score(&brick_score, 10);
                    combo_timeout = 0;
                    goto collision_done;
                }
            }
        }
        collision_done:;
    }

    // combo超时
    combo_timeout++;
    if (combo_timeout >= BRICK_COMBO_TIMEOUT && brick_score.combo_count > 0) {
        game_common_combo_reset(&brick_score);
    }
    game_common_combo_display_tick(&brick_combo_display);

    // 通关检测
    if (bricks_remaining <= 0) {
        game_common_add_score(&brick_score, 50);
        brick_scene = SCENE_DYING;
        dying_timer = 0;
        return;
    }

    // 渲染
    port_clear_screen();
    draw_bricks();
    draw_paddle();
    draw_ball();
    game_common_combo_display_render(&brick_combo_display, brick_score.current_score);
    port_update_screen();
}

static void brick_dying_tick(void) {
    dying_timer++;
    if (dying_timer >= BRICK_DYING_FRAMES) {
        game_common_save_if_needed(STORAGE_ID_BRICK, &brick_score);
        brick_scene = SCENE_RESULT;
        scene_frame = 0;
        return;
    }
    // 闪烁效果
    port_clear_screen();
    if (dying_timer % 3 != 0) {
        draw_bricks();
        draw_paddle();
    }
    port_draw_num(93, 2, brick_score.current_score, 5, FONT_SMALL);
    port_update_screen();
}

static void brick_result_tick(void) {
    scene_frame++;

    if (scene_frame > 40 && (port_encoder_get() != 0 || click_input)) {
        click_input = false;
        brick_game_init();
        return;
    }

    game_common_result_render(&brick_score, scene_frame);
}

void brick_game_tick(void) {
    switch (brick_scene) {
        case SCENE_TITLE:
            brick_title_tick();
            break;
        case SCENE_RUNNING:
            brick_running_tick();
            break;
        case SCENE_DYING:
            brick_dying_tick();
            break;
        case SCENE_RESULT:
            brick_result_tick();
            break;
    }
}

void brick_game_on_exit(void) {
    port_clear_screen();
    port_update_screen();
}

void brick_game_loop(void) {
    brick_game_init();

    while (1) {
        if (exit_requested) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        brick_game_tick();
        port_delay_ms(30);
    }
}
