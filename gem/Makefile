.PHONY: all
all:
	rake compile

.PHONY: debug
debug:
	GEM_DEBUG=1 rake compile

# touch the source to force rake compile to compile everytime
.PHONY: touch
touch:
	touch ext/alt_printf/alt_printf.c

.PHONY: clean
clean:
	rm -f pkg/*.gem
	rm -rf tmp
	rm -f lib/alt_printf/alt_printf.so
