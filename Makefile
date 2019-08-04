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
CFLAGS += $($(TARGET)_cflags)

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
	rm -f ext/alt_printf.o ext/alt_printf.so ext/Makefile ext/mkmf.log ext/extconf.h

.PHONY: hello
hello: ARGS = "hello world"
hello: run

.PHONY: run
run: all
	@$(target_dir)/altprintf $(ARGS)

.PHONY: test
test: all
	bundle exec rspec

ext/Makefile: ext/extconf.rb
	cd ext/ && ruby extconf.rb

.PHONY: ruby
ruby: $(objects) ext/Makefile
	make -C ext/
