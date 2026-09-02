#include "project.h"

int points_equal(Point a, Point b) {
    return (a.x == b.x && a.y == b.y);
}

void place_food(GameState *gs) {
    Point free_cells[BOARD_W * BOARD_H];
    int free_count = 0;

    for (int x = 0; x < BOARD_W; x++) {
        for (int y = 0; y < BOARD_H; y++) {
            Point p = {x, y};
            int occupied = 0;
            for (int i = 0; i < gs->length; i++) {
                if (points_equal(gs->snake[i], p)) { occupied = 1; break; }
            }
            if (!occupied) free_cells[free_count++] = p;
        }
    }

    if (free_count == 0) { gs->food.x = -1; gs->food.y = -1; return; }
    gs->food = free_cells[rand() % free_count];
}

void init_game(GameState *gs) {
    memset(gs, 0, sizeof(GameState));
    gs->length   = 3;
    gs->dir      = DIR_RIGHT;
    gs->alive    = 1;
    gs->score    = 0;
    gs->delay_us = INITIAL_DELAY_US;

    gs->snake[0].x = 4; gs->snake[0].y = 3;
    gs->snake[1].x = 3; gs->snake[1].y = 3;
    gs->snake[2].x = 2; gs->snake[2].y = 3;

    srand((unsigned int)time(NULL));
    place_food(gs);
}

void step_game(GameState *gs) {
    if (!gs->alive) return;

    Point new_head = gs->snake[0];
    switch (gs->dir) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
        default: break;
    }

    if (new_head.x < 0 || new_head.x >= BOARD_W ||
        new_head.y < 0 || new_head.y >= BOARD_H) {
        gs->alive = 0; return;
    }

    for (int i = 0; i < gs->length - 1; i++) {
        if (points_equal(new_head, gs->snake[i])) { gs->alive = 0; return; }
    }

    int ate = points_equal(new_head, gs->food);

    if (ate) {
        if (gs->length < MAX_SNAKE) {
            for (int i = gs->length; i > 0; i--)
                gs->snake[i] = gs->snake[i-1];
            gs->length++;
        }
        gs->score++;
        gs->delay_us -= SPEED_STEP_US;
        if (gs->delay_us < MIN_DELAY_US) gs->delay_us = MIN_DELAY_US;
        place_food(gs);
    } else {
        for (int i = gs->length - 1; i > 0; i--)
            gs->snake[i] = gs->snake[i-1];
    }

    gs->snake[0] = new_head;
}

static int is_opposite(Direction cur, Direction nd) {
    if (nd == DIR_NONE) return 0;
    if (cur == DIR_UP    && nd == DIR_DOWN)  return 1;
    if (cur == DIR_DOWN  && nd == DIR_UP)    return 1;
    if (cur == DIR_LEFT  && nd == DIR_RIGHT) return 1;
    if (cur == DIR_RIGHT && nd == DIR_LEFT)  return 1;
    return 0;
}

int main(void) {
    int evfd    = open_joystick();
    int fbfd    = open_framebuffer();
    int accelfd = open_accelerometer();

    if (evfd < 0)  { fprintf(stderr, "Joystick not found\n");    return 1; }
    if (fbfd <= 0) { fprintf(stderr, "Framebuffer not found\n"); return 1; }

    Fb *fb = mmap(0, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (!fb) { fprintf(stderr, "mmap failed\n"); return 1; }

    GameState gs;

    while (1) {
        init_game(&gs);
        render_game(fb, &gs);

        while (gs.alive) {
            Direction nd = read_direction(evfd, accelfd);
            if (nd != DIR_NONE && !is_opposite(gs.dir, nd))
                gs.dir = nd;

            step_game(&gs);
            render_game(fb, &gs);
            usleep(gs.delay_us);
        }

        render_game_over(fb, &gs);
        printf("Game over! Score: %d\n", gs.score);

        wait_for_restart(evfd);
        clear_display(fb);
    }

    munmap(fb, 128);
    close(fbfd);
    close(evfd);
    if (accelfd >= 0) close(accelfd);
    return 0;
}
