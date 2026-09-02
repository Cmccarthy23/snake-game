#include "project.h"
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define LSM9DS1_ADDR   0x6a
#define CTRL_REG6_XL   0x20
#define OUT_X_L_XL     0x28

static int i2c_fd = -1;

static int init_accelerometer(void) {
    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) return -1;

    if (ioctl(i2c_fd, I2C_SLAVE, LSM9DS1_ADDR) < 0) return -1;

    uint8_t cmd[2] = { CTRL_REG6_XL, 0x60 };
    if (write(i2c_fd, cmd, 2) != 2) return -1;

    return 0;
}

static int read_accel(int16_t *ax, int16_t *ay) {
    uint8_t reg = OUT_X_L_XL | 0x80;
    uint8_t buf[4];

    if (write(i2c_fd, &reg, 1) != 1) return -1;
    if (read(i2c_fd, buf, 4) != 4)   return -1;

    *ax = (int16_t)(buf[1] << 8 | buf[0]);
    *ay = (int16_t)(buf[3] << 8 | buf[2]);
    return 0;
}

int open_joystick(void) {
    struct dirent **namelist;
    int i, ndev, fd = -1;

    ndev = scandir(DEV_INPUT_EVENT, &namelist, NULL, versionsort);
    if (ndev <= 0) return -1;

    for (i = 0; i < ndev; i++) {
        char fname[64], name[256];
        snprintf(fname, sizeof(fname), "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
        fd = open(fname, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        if (strcmp("Raspberry Pi Sense HAT Joystick", name) == 0) break;
        close(fd);
        fd = -1;
    }
    for (i = 0; i < ndev; i++) free(namelist[i]);
    free(namelist);
    return fd;
}

int open_framebuffer(void) {
    struct dirent **namelist;
    int i, ndev, fd = -1;
    struct fb_fix_screeninfo fix_info;

    ndev = scandir(DEV_FB, &namelist, NULL, versionsort);
    if (ndev <= 0) return -1;

    for (i = 0; i < ndev; i++) {
        char fname[64];
        snprintf(fname, sizeof(fname), "%s/%s", DEV_FB, namelist[i]->d_name);
        fd = open(fname, O_RDWR);
        if (fd < 0) continue;
        ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);
        if (strcmp("RPi-Sense FB", fix_info.id) == 0) break;
        close(fd);
        fd = -1;
    }
    for (i = 0; i < ndev; i++) free(namelist[i]);
    free(namelist);
    return fd;
}

int open_accelerometer(void) {
    return init_accelerometer();
}

Direction read_joystick_direction(int evfd) {
    struct input_event ev[64];
    int i, rd;

    rd = read(evfd, ev, sizeof(ev));
    if (rd < (int)sizeof(struct input_event)) return DIR_NONE;

    for (i = 0; i < rd / (int)sizeof(struct input_event); i++) {
        if (ev[i].type != EV_KEY || ev[i].value != 1) continue;
        switch (ev[i].code) {
            case KEY_UP:    return DIR_UP;
            case KEY_DOWN:  return DIR_DOWN;
            case KEY_LEFT:  return DIR_LEFT;
            case KEY_RIGHT: return DIR_RIGHT;
            default: break;
        }
    }
    return DIR_NONE;
}

Direction read_tilt_direction(int accelfd) {
    int16_t ax, ay;
    (void)accelfd;

    if (i2c_fd < 0) return DIR_NONE;
    if (read_accel(&ax, &ay) < 0) return DIR_NONE;

    int abs_ax = ax < 0 ? -ax : ax;
    int abs_ay = ay < 0 ? -ay : ay;

    if (abs_ax < TILT_THRESHOLD && abs_ay < TILT_THRESHOLD) return DIR_NONE;

    if (abs_ax >= abs_ay)
        return (ax > 0) ? DIR_LEFT : DIR_RIGHT;
    else
        return (ay > 0) ? DIR_UP : DIR_DOWN;
}

Direction read_direction(int evfd, int accelfd) {
    Direction tilt = read_tilt_direction(accelfd);
    if (tilt != DIR_NONE) return tilt;
    return read_joystick_direction(evfd);
}

void wait_for_restart(int evfd) {
    struct input_event ev[64];
    int i, rd;

    while (1) {
        struct pollfd pfd = { .fd = evfd, .events = POLLIN };
        if (poll(&pfd, 1, 100) <= 0) continue;

        rd = read(evfd, ev, sizeof(ev));
        if (rd < (int)sizeof(struct input_event)) continue;

        for (i = 0; i < rd / (int)sizeof(struct input_event); i++) {
            if (ev[i].type == EV_KEY &&
                ev[i].code == KEY_ENTER &&
                ev[i].value == 1)
                return;
        }
    }
}
