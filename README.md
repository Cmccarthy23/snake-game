# cisc210-final – Snake on Raspberry Pi Sense Hat

## Partners

| Name | Student ID |
|------|------------|
| CAIDEN MCCARTHY | 702790987 |

## Contributions

Solo project – all code written by the student listed above.

## Project Description

A fully playable Snake game running on a Raspberry Pi with a Sense Hat.
The 8x8 LED array is the display; the joystick and accelerometer are the
two input sources. The snake grows each time it eats food (red pixel),
and the game ends when the snake hits a wall or itself. Speed increases
with every food item eaten.

## Usage Instructions

### Build
```bash
make
```

### Run
```bash
./snake
```

### Controls
- Tilt the Pi to steer (priority)
- Joystick Up/Down/Left/Right to steer (fallback)
- Joystick centre button to restart after game over

### Display
- Bright green = snake head
- Dark green = snake body
- Red = food
- Orange flash = game over

### Clean
```bash
make clean
```
