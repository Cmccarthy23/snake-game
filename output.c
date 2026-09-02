#include "project.h"

void clear_display(Fb *fb) {
    memset(fb, 0, 128);
}

void render_game(Fb *fb, const GameState *gs) {
    clear_display(fb);

    if (gs->food.x >= 0)
        fb->pixel[gs->food.y][gs->food.x] = COLOR_FOOD;

    for (int i = 1; i < gs->length; i++)
        fb->pixel[gs->snake[i].y][gs->snake[i].x] = COLOR_BODY;

    fb->pixel[gs->snake[0].y][gs->snake[0].x] = COLOR_HEAD;
}

void render_game_over(Fb *fb, const GameState *gs) {
    int f;
    for (f = 0; f < 3; f++) {
        clear_display(fb);
        for (int i = 0; i < gs->length; i++)
            fb->pixel[gs->snake[i].y][gs->snake[i].x] = COLOR_DEAD;
        usleep(120000);

        clear_display(fb);
        usleep(100000);
    }

    for (int i = 1; i < gs->length; i++)
        fb->pixel[gs->snake[i].y][gs->snake[i].x] = 0x0300;
    fb->pixel[gs->snake[0].y][gs->snake[0].x] = COLOR_DEAD;
}
