#ifndef PROJECT_H
#define PROJECT_H

#define _GNU_SOURCE
#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME  "event"
#define DEV_FB          "/dev"
#define FB_DEV_NAME     "fb"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <poll.h>
#include <dirent.h>
#include <string.h>
#include <math.h>

#define BOARD_W  8
#define BOARD_H  8
#define TILT_THRESHOLD   8000
#define INITIAL_DELAY_US  350000
#define MIN_DELAY_US       80000
#define SPEED_STEP_US      20000

#define COLOR_BLACK  0x0000
#define COLOR_HEAD   0x07E0
#define COLOR_BODY   0x03E0
#define COLOR_FOOD   0xF800
#define COLOR_DEAD   0xFC00

typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE } Direction;
typedef struct { uint16_t pixel[8][8]; } Fb;

#define MAX_SNAKE 64
typedef struct { int x, y; } Point;

typedef struct {
    Point     snake[MAX_SNAKE];
    int       length;
    Direction dir;
    Point     food;
    int       alive;
    int       score;
    int       delay_us;
} GameState;

/* input.c */
int       open_joystick(void);
int       open_framebuffer(void);
int       open_accelerometer(void);
Direction read_joystick_direction(int evfd);
Direction read_tilt_direction(int accelfd);
Direction read_direction(int evfd, int accelfd);
void      wait_for_restart(int evfd);

/* output.c */
void render_game(Fb *fb, const GameState *gs);
void render_game_over(Fb *fb, const GameState *gs);
void clear_display(Fb *fb);

/* main.c */
void init_game(GameState *gs);
void place_food(GameState *gs);
void step_game(GameState *gs);
int  points_equal(Point a, Point b);

#endif
