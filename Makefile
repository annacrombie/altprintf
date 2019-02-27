objs = $(patsubst src/%.c,build/release_%.o,$(wildcard src/*.c))
dbg_objs = $(patsubst src/%.c,build/debug_%.o,$(wildcard src/*.c))
headers := $(wildcard src/*.h)
source := $(wildcard src/*.c)
DEBUG := -g -D DEBUG
RELEASE := -O2

all: altprintf

build:
	mkdir build

build/debug_%.o: build src/%.h src/%.c
	gcc $(DEBUG) -o build/debug_$*.o -c src/$*.c

build/release_%.o: build src/%.h src/%.c
	gcc $(RELEASE) -o build/release_$*.o -c src/$*.c

altprintf_debug: src/cli.c $(dbg_objs)
	@echo $(objs)
	gcc $(DEBUG) -lm -o altprintf build/debug_*.o

altprintf: $(objs)
	gcc $(RELEASE) -lm -o altprintf build/release_*.o
	strip altprintf

clean:
	rm -rf build/*.o altprintf altprintf_debug

run: altprintf_debug
	@./altprintf "test %s" string

test: altprintf
