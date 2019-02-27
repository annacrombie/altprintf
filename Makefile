objs = $(patsubst src/%.c,build/release_%.o,$(wildcard src/*.c))
dbg_objs = $(patsubst src/%.c,build/debug_%.o,$(wildcard src/*.c))
headers := $(wildcard src/*.h)
source := $(wildcard src/*.c)
DEBUG := -g -D DEBUG
RELEASE := -O2

all: altprintf

build/debug_%.o: src/%.h src/%.c
	mkdir -p build
	gcc $(DEBUG) -o build/debug_$*.o -c src/$*.c

build/release_%.o: src/%.h src/%.c
	mkdir -p build
	gcc $(RELEASE) -o build/release_$*.o -c src/$*.c

altprintf_debug: src/cli.c $(dbg_objs)
	gcc $(DEBUG) -lm -o altprintf_debug build/debug_*.o

altprintf: src/cli.h $(objs)
	gcc $(RELEASE) -lm -o altprintf build/release_*.o

clean:
	rm -rf build/*.o altprintf altprintf_debug

run: altprintf_debug
	@./altprintf "test %s" string

test: altprintf
	bundle exec rspec
