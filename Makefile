MAKEFLAGS += -j2 -rR --include-dir=$(CURDIR)
CC = gcc

TARGET ?= release
target_dir := target/$(TARGET)
objects := \
  $(patsubst src/%.c,$(target_dir)/%.o,$(wildcard src/*.c))


debug_cflags := -g -D DEBUG
release_cflags := -O2
CFLAGS += $($(TARGET)_cflags)

all: $(target_dir)/altprintf

.PRECIOUS: %.o
%.o: ../../src/%.c
	$(CC) $(CFLAGS) -o $*.o -c $(subst $(target_dir),src,$*).c

%/altprintf: $(objects)
	$(CC) $(CFLAGS) -lm -o $*/altprintf $(objects)

.PHONY: clean
clean:
	find target -type f -name \*.o -or -name altprintf \
	  | xargs rm

.PHONY: hello
hello: ARGS = "hello world"
hello: run

.PHONY: run
run: all
	@$(target_dir)/altprintf $(ARGS)

.PHONY: test
test: target/release/altprintf
	bundle exec rspec
