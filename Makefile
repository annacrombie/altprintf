MAKEFLAGS += -rR --include-dir=$(CURDIR)

TARGET ?= release
target_dir := target/$(TARGET)
objects := \
  $(patsubst src/%.c,$(target_dir)/%.o,$(wildcard src/*.c))

CC = gcc


all: $(TARGET)

$(target_dir):
	mkdir -p $(target_dir)

%.o: ../../src/%.c | $(target_dir)
	$(CC) $(CFLAGS) -o $*.o -c $(subst $(target_dir),src,$*).c

%/altprintf: $(objects)
	$(CC) $(CFLAGS) -lm -o $*/altprintf $(objects)

.PHONY: clean
clean:
	rm target/release/*
	touch target/release/.keep
	rm target/debug/*
	touch target/debug/.keep

.PHONY: debug
debug: CFLAGS += -g -D DEBUG
debug: $(target_dir)/altprintf

.PHONY: release
release: CFLAGS += -O2
release: $(target_dir)/altprintf

.PHONY: release
run: $(TARGET)
	@$(target_dir)/altprintf "hello world"

.PHONY: test
test: release
	bundle exec rspec
