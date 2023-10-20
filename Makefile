cc := gcc

srcs := $(wildcard src/*.c) $(wildcard src/**/*.c)
objs := $(patsubst %.c,%.o,$(srcs))

cflags := -Wall -Wextra -Wpedantic -std=c17 -g
cflags += $(shell pkg-config --cflags raylib)
cflags += -Ilibs/raygui/src -Iinclude/

ldflags := -lm
ldflags += $(shell pkg-config --libs raylib)

bin := chip8

.PHONY: clean run

all: $(bin)

$(bin): $(objs)
	$(cc) $(ldflags) $^ -o $@

%.o: %.c
	$(cc) $(cflags) -c $< -o $@

run: all
	./$(bin)

clean:
	rm -rf $(bin) $(objs)
