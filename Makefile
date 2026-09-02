CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g
LIBS    = -lm

TARGET  = snake
SRCS    = main.c input.c output.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c project.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
