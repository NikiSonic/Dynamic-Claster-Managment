# make — собрать в build/node, make clean — убрать build

CC     = gcc
CFLAGS = -Wall -Wextra -g

SRC = $(wildcard src/*.c)
HDR = $(wildcard src/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

build/node: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# .o зависит от всех .h: поменял заголовок — пересоберётся всё
build/%.o: src/%.c $(HDR) | build
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: clean
