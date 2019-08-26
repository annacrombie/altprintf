MAKEFLAGS += -j2 -rR --include-dir=$(CURDIR)
CC = gcc

ifeq (release,$(MAKECMDGOALS))
  target := release
  MAKECMDGOALS := all
endif
ifeq (debug,$(MAKECMDGOALS))
  target := debug
  MAKECMDGOALS := all
endif

target ?= release
TARGET ?= $(target)
target_dir := target/$(TARGET)
objects := \
  $(patsubst src/%.c,$(target_dir)/%.o,$(wildcard src/*.c))

debug_cflags := -g -D DEBUG
release_cflags := -O2
CFLAGS += -Wextra -Wall $($(TARGET)_cflags)

.PHONY: all debug release
all debug release: $(objects) $(target_dir)/altprintf

$(target_dir):
	mkdir -p $(target_dir)

.PRECIOUS: %.o
.SECONDEXPANSION:
%.o: src/$$(basename $$(notdir $$@)).c | $(target_dir)
	$(CC) $(CFLAGS) -o $*.o -c $(subst $(target_dir),src,$*).c

%/altprintf: $(objects)
	$(CC) $(CFLAGS) -lm -o $*/altprintf $(objects)

.PHONY: clean
clean:
	rm -rf target
	make -C gem clean

.PHONY: hello
hello: ARGS = "hello world"
hello: run

.PHONY: run
run: all
	@$(target_dir)/altprintf $(ARGS)

.PHONY: test
test: all gem
	bundle exec rspec

.PHONY: touchgem
touchgem:
	touch gem/ext/altprintf/altprintf.c

.PHONY: gem
gem: release
	make -C gem/

.PHONY: gemd
gemd: debug
	make -C gem/ debug
