.PHONY: all
all:
	rake compile

.PHONY: debug
debug:
	GEM_DEBUG=1 rake compile

# touch the source to force rake compile to compile everytime
.PHONY: touch
touch:
	touch ext/altprintf/altprintf.c

.PHONY: clean
clean:
	rm -rf tmp
	rm -f lib/altprintf/altprintf.so
