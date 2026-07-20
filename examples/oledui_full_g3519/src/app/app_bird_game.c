/**
 * app_bird_game.c
 * 水管鸟游戏实现
 */

#include "app_bird_game.h"
#include "app_game_common.h"
#include "game_port.h"
#include "stdlib.h"
#include "OLED.h"
#include "OLED_UI_Driver.h"
#include "hw_delay.h"

#define BIRD_DYING_FRAMES  15

static game_scene_t bird_scene = SCENE_TITLE;
static bird_game_object_t bird;
static pipe_pair_t pipes[MAX_PIPES];
static unsigned int score = 0;
static unsigned int animation_frame = 0;
static bool jump_requested = false;
static bool exit_requested = false;
static unsigned int background_offset = 0;
static const unsigned int BACKGROUND_WIDTH = 128;

static game_score_t bird_score;
static game_combo_display_t bird_combo_display = {0, 0, false};
static uint16_t scene_frame = 0;
static uint16_t dying_timer = 0;

// 前向声明
static void draw_bird(unsigned int x, unsigned int y, unsigned int frame);
static void draw_pipe(bird_game_object_t *pipe);
static void draw_background(void);
static void generate_pipe(unsigned int index);
static void pregenerate_initial_pipes(void);
static void game_handle_input(void);
static void game_update(void);
static void game_render(void);
static bool game_check_collision(void);
static void bird_title_tick(void);
static void bird_dying_tick(void);
static void bird_result_tick(void);

void game_init(void)
{
    srand(animation_frame);
    bird_scene = SCENE_TITLE;
    animation_frame = 0;
    jump_requested = false;
    exit_requested = false;
    background_offset = 0;
    scene_frame = 0;
    dying_timer = 0;

    bird.x = 30;
    bird.y = SCREEN_HEIGHT / 2;
    bird.width = BIRD_WIDTH;
    bird.height = BIRD_HEIGHT;
    bird.y_speed = 0;

    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        pipes[i].top.width = 0;
        pipes[i].bottom.width = 0;
        pipes[i].passed = false;
    }

    game_common_score_reset(&bird_score);
    game_common_load_save(STORAGE_ID_BIRD, &bird_score);
    bird_combo_display.multiplier = 0;
    bird_combo_display.display_timer = 0;
    bird_combo_display.score_flash = false;
    score = 0;
}

void game_set_jump(void)
{
    jump_requested = true;
}

void game_start(void)
{
    game_loop();
}

void game_loop(void)
{
    game_init();

    while (1) {
        if (game_should_exit()) {
            port_clear_screen();
            port_update_screen();
            break;
        }

        switch (bird_scene) {
            case SCENE_TITLE:
                bird_title_tick();
                break;
            case SCENE_RUNNING:
                game_handle_input();
                game_update();
                game_render();
                break;
            case SCENE_DYING:
                bird_dying_tick();
                break;
            case SCENE_RESULT:
                bird_result_tick();
                break;
        }

        port_delay_ms(30);
    }
}

void game_request_exit(void)
{
    exit_requested = true;
}

bool game_should_exit(void)
{
    return exit_requested;
}

void game_tick(void)
{
    switch (bird_scene) {
        case SCENE_TITLE:
            bird_title_tick();
            break;
        case SCENE_RUNNING:
            game_handle_input();
            game_update();
            game_render();
            break;
        case SCENE_DYING:
            bird_dying_tick();
            break;
        case SCENE_RESULT:
            bird_result_tick();
            break;
    }
}

void game_on_exit(void)
{
    port_clear_screen();
    port_update_screen();
}

// SCENE_TITLE
static void bird_title_tick(void)
{
    scene_frame++;
    animation_frame++;

    if (port_encoder_get() != 0 || jump_requested) {
        jump_requested = false;
        bird_scene = SCENE_RUNNING;
        scene_frame = 0;
        bird.y_speed = JUMP_FORCE;
        pregenerate_initial_pipes();
        return;
    }

    port_clear_screen();

    // 绘制地面
    port_draw_line(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 5);
    port_draw_line(0, SCREEN_HEIGHT - 4, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 4);

    // 小鸟原地扇翅膀
    draw_bird(bird.x, bird.y, animation_frame);

    game_common_title_render("FLAPPY BIRD", 10, bird_score.high_score, scene_frame);
    port_update_screen();
}

// SCENE_DYING
static void bird_dying_tick(void)
{
    dying_timer++;

    // 小鸟坠落动画
    bird.y_speed += GRAVITY * 2;
    bird.y += bird.y_speed;
    if (bird.y > SCREEN_HEIGHT) bird.y = SCREEN_HEIGHT;

    if (dying_timer >= BIRD_DYING_FRAMES) {
        game_common_save_if_needed(STORAGE_ID_BIRD, &bird_score);
        bird_scene = SCENE_RESULT;
        scene_frame = 0;
        return;
    }

    // 渲染冻结画面 + 坠落的鸟
    draw_background();
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width > 0 && pipes[i].top.x < SCREEN_WIDTH) {
            draw_pipe(&pipes[i].top);
            draw_pipe(&pipes[i].bottom);
        }
    }
    draw_bird(bird.x, bird.y, animation_frame);
    port_update_screen();
}

// SCENE_RESULT
static void bird_result_tick(void)
{
    scene_frame++;

    if (scene_frame > 40 && (port_encoder_get() != 0 || jump_requested)) {
        jump_requested = false;
        game_init();
        return;
    }

    game_common_result_render(&bird_score, scene_frame);
}

// 绘制小鸟
static void draw_bird(unsigned int x, unsigned int y, unsigned int frame)
{
    OLED_ClearArea(x - BIRD_WIDTH/2 - 2, y - BIRD_HEIGHT/2 - 2,
                  BIRD_WIDTH + 4, BIRD_HEIGHT + 4);

    OLED_DrawEllipse(x, y, 4, 3, OLED_UNFILLED);
    OLED_ClearArea(x + 2, y - 2, 2, 2);
    OLED_DrawPoint(x + 2, y - 2);
    OLED_DrawTriangle(x + 4, y - 1, x + 4, y + 1, x + 7, y, OLED_UNFILLED);
    OLED_DrawLine(x - 4, y - 1, x - 6, y - 3);
    OLED_DrawLine(x - 4, y, x - 6, y - 1);

    if (frame % 8 < 4) {
        OLED_DrawTriangle(x - 2, y - 1, x + 1, y - 1, x - 1, y - 5, OLED_UNFILLED);
    } else {
        OLED_DrawTriangle(x - 2, y + 1, x + 1, y + 1, x - 1, y + 4, OLED_UNFILLED);
    }
}

// 绘制管道
static void draw_pipe(bird_game_object_t *pipe)
{
    if (pipe->x + pipe->width <= 0 || pipe->x >= SCREEN_WIDTH || pipe->width == 0) {
        return;
    }

    OLED_DrawRectangle(pipe->x, pipe->y, pipe->width, pipe->height, OLED_FILLED);

    if (pipe->y == 0) {
        OLED_DrawRectangle(pipe->x - 1, pipe->y + pipe->height - 1,
                          pipe->width + 2, 2, OLED_FILLED);
    } else {
        OLED_DrawRectangle(pipe->x - 1, pipe->y,
                          pipe->width + 2, 2, OLED_FILLED);
    }
}

// 绘制背景
static void draw_background(void)
{
    OLED_Clear();

    OLED_DrawLine(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 5);
    OLED_DrawLine(0, SCREEN_HEIGHT - 4, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 4);

    for (int i = -8 + (int)(background_offset % 8); i < (int)SCREEN_WIDTH; i += 8) {
        OLED_DrawLine(i, SCREEN_HEIGHT - 3, i + 3, SCREEN_HEIGHT - 1);
    }

    int cloud_offset = (int)(background_offset / 3) % SCREEN_WIDTH;

    int cx1 = (40 - cloud_offset + SCREEN_WIDTH) % SCREEN_WIDTH;
    OLED_DrawEllipse(cx1, 8, 8, 3, OLED_FILLED);
    OLED_DrawEllipse(cx1 - 6, 9, 4, 2, OLED_FILLED);
    OLED_DrawEllipse(cx1 + 6, 9, 5, 2, OLED_FILLED);

    int cx2 = (110 - cloud_offset + SCREEN_WIDTH) % SCREEN_WIDTH;
    OLED_DrawEllipse(cx2, 14, 6, 2, OLED_FILLED);
    OLED_DrawEllipse(cx2 - 5, 15, 3, 2, OLED_FILLED);
    OLED_DrawEllipse(cx2 + 5, 15, 4, 2, OLED_FILLED);

    background_offset = (background_offset + 1) % BACKGROUND_WIDTH;
}

// 生成管道
static void generate_pipe(unsigned int index)
{
    if (index >= MAX_PIPES) return;

    unsigned int min_gap = PIPE_GAP_HEIGHT / 2 + 10;
    unsigned int max_gap = SCREEN_HEIGHT - 4 - PIPE_GAP_HEIGHT / 2 - 10;
    unsigned int gap_center = min_gap + (rand() % (max_gap - min_gap + 1));

    unsigned int start_x = SCREEN_WIDTH + 10 + (index * (SCREEN_WIDTH / 2));

    pipes[index].top.x = start_x;
    pipes[index].top.y = 0;
    pipes[index].top.width = PIPE_WIDTH;
    pipes[index].top.height = gap_center - PIPE_GAP_HEIGHT / 2;
    pipes[index].top.y_speed = PIPE_SPEED;

    pipes[index].bottom.x = start_x;
    pipes[index].bottom.y = gap_center + PIPE_GAP_HEIGHT / 2;
    pipes[index].bottom.width = PIPE_WIDTH;
    pipes[index].bottom.height = SCREEN_HEIGHT - pipes[index].bottom.y - 4;
    pipes[index].bottom.y_speed = PIPE_SPEED;

    pipes[index].passed = false;
}

static void pregenerate_initial_pipes(void)
{
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        generate_pipe(i);
    }
}

// 处理输入
static void game_handle_input(void)
{
    if (Encoder_Get() != 0) {
        jump_requested = true;
    }

    if (jump_requested) {
        jump_requested = false;
        if (bird_scene == SCENE_RUNNING) {
            bird.y_speed = JUMP_FORCE;
        }
    }
}

// 更新游戏状态
static void game_update(void)
{
    if (bird_scene != SCENE_RUNNING) return;

    animation_frame++;

    bird.y_speed += GRAVITY;
    bird.y += bird.y_speed;

    if (bird.y < BIRD_HEIGHT / 2) {
        bird.y = BIRD_HEIGHT / 2;
        bird.y_speed = 0;
    } else if (bird.y > SCREEN_HEIGHT - 4 - BIRD_HEIGHT / 2) {
        bird_scene = SCENE_DYING;
        dying_timer = 0;
        return;
    }

    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width <= 0) {
            generate_pipe(i);
        }

        pipes[i].top.x -= pipes[i].top.y_speed;
        pipes[i].bottom.x -= pipes[i].bottom.y_speed;

        if (!pipes[i].passed && pipes[i].top.x + pipes[i].top.width < bird.x) {
            pipes[i].passed = true;
            game_common_combo_hit(&bird_score, &bird_combo_display);
            game_common_add_score(&bird_score, 1);
            score++;
        }

        if (pipes[i].top.x <= 0) {
            generate_pipe(i);
            unsigned int max_x = SCREEN_WIDTH + 10;
            for (unsigned int j = 0; j < MAX_PIPES; j++) {
                if (j != i && pipes[j].top.x > max_x) {
                    max_x = pipes[j].top.x;
                }
            }
            pipes[i].top.x = max_x + (SCREEN_WIDTH / 3);
            pipes[i].bottom.x = pipes[i].top.x;
        }
    }

    game_common_combo_display_tick(&bird_combo_display);

    if (game_check_collision()) {
        bird_scene = SCENE_DYING;
        dying_timer = 0;
    }
}

// 渲染
static void game_render(void)
{
    if (bird_scene != SCENE_RUNNING) return;

    draw_background();

    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width > 0 && pipes[i].top.x < SCREEN_WIDTH) {
            draw_pipe(&pipes[i].top);
            draw_pipe(&pipes[i].bottom);
        }
    }

    draw_bird(bird.x, bird.y, animation_frame);
    game_common_combo_display_render(&bird_combo_display, bird_score.current_score);
    OLED_Update();
}

// 碰撞检测
static bool game_check_collision(void)
{
    for (unsigned int i = 0; i < MAX_PIPES; i++) {
        if (pipes[i].top.width > 0) {
            int bird_left = bird.x - BIRD_WIDTH / 2;
            int bird_right = bird.x + BIRD_WIDTH / 2;
            int bird_top = bird.y - BIRD_HEIGHT / 2;
            int bird_bottom = bird.y + BIRD_HEIGHT / 2;

            if (bird_right > (int)pipes[i].top.x && bird_left < (int)(pipes[i].top.x + pipes[i].top.width) &&
                bird_bottom > (int)pipes[i].top.y && bird_top < (int)(pipes[i].top.y + pipes[i].top.height)) {
                return true;
            }

            if (bird_right > (int)pipes[i].bottom.x && bird_left < (int)(pipes[i].bottom.x + pipes[i].bottom.width) &&
                bird_bottom > (int)pipes[i].bottom.y && bird_top < (int)(pipes[i].bottom.y + pipes[i].bottom.height)) {
                return true;
            }
        }
    }
    return false;
}
