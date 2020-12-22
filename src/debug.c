#include "posix.h"

#include <assert.h>
#include <stdarg.h>  // debug
#include <string.h>  // memcpy

#include "apf.h"
#include "common.h"
#include "debug.h"

bool apf_verbose = false;

static void
tree_printer(uint32_t depth, const char *fmt, ...)
{
	uint32_t i;
	for (i = 0; i < depth; ++i) {
		putc(' ', stderr);
		putc(' ', stderr);
		putc(' ', stderr);
		putc(' ', stderr);
	}

	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void
print_n(const uint8_t *start, uint32_t len)
{
	char buf[128] = { 0 };
	memcpy(buf, start, len > 127 ? 127 : len);
	tree_printer(0, "%s", buf);
}

void
apf_dbg_print_parse_tree(struct apf_template *apft, uint32_t depth, uint32_t orig_i)
{
	uint32_t i, len;
	uint8_t dat_hdr;
	uint16_t id_hdr;
	uint16_t cond_hdr[2];

	if (!depth) {
		L("parse tree:\n");
	}

	for (i = 0; i < apft->len; ++i) {
		tree_printer(depth, "(%3d,%3d):", i, i + orig_i);
		switch (apft->elem[i] & 0x1) {
		case apft_dat:
			tree_printer(0, "dat:");
			dat_hdr = apft->elem[i];
			memcpy(&id_hdr, &apft->elem[i + 1], 2);

			i += 3;

			if (dat_hdr & apff_conditional) {
				tree_printer(0, "cond:id:");

				switch (id_hdr & 0x3) {
				case apft_id_num:
					tree_printer(0, "num:%d", id_hdr >> 2);
					break;
				case apft_id_sym:
					len = id_hdr >> 2;
					tree_printer(0, "sym:");
					print_n(&apft->elem[i], len);
					tree_printer(0, ":%p", (void *)&apft->elem[i]);
					i += len;
					break;
				default:
					assert(false);
					break;
				}

				memcpy(cond_hdr, &apft->elem[i], 4);
				i += 4;

				tree_printer(0,  "\n");
				tree_printer(depth, "  arm:%d\n", cond_hdr[0]);
				{
					struct apf_template sub = { .elem = &apft->elem[i], .len = cond_hdr[0] };
					apf_dbg_print_parse_tree(&sub, depth + 1, i);
				}
				tree_printer(depth, "  arm:%d\n", cond_hdr[1]);
				{
					struct apf_template sub = { .elem = &apft->elem[i + cond_hdr[0]], .len = cond_hdr[1] };
					apf_dbg_print_parse_tree(&sub, depth + 1, i);
				}
				i += cond_hdr[0] + cond_hdr[1];
			} else {
				tree_printer(0, "basic:id:");
				switch (id_hdr & 0x3) {
				case apft_id_num:
					tree_printer(0, "num:%d\n", id_hdr >> 2);
					break;
				case apft_id_sym:
					len = id_hdr >> 2;
					tree_printer(0, "sym:");
					print_n(&apft->elem[i], len);
					tree_printer(0, ":%p", (void *)&apft->elem[i]);
					tree_printer(0, "\n");
					i += len;
					break;
				case apft_id_lit:
					len = id_hdr >> 2;
					tree_printer(0, "lit:%d:%p\n", len, (void *)&apft->elem[i]);
					struct apf_template sub = { .elem = &apft->elem[i], .len = len };
					apf_dbg_print_parse_tree(&sub, depth + 1, i);
					i += len;
					break;
				default:
					assert(false);
					break;
				}

				i += ((dat_hdr & apff_align_chr) ? 1 : 0)
				     + ((dat_hdr & apff_width) ? 1 : 0)
				     + ((dat_hdr & apff_prec) ? 1 : 0)
				     + ((dat_hdr & apff_trans) ? 1 : 0);
			}

			i -= 1;

			break;
		case apft_raw:
			len = apft->elem[i] >> 1;
			tree_printer(0, "raw:%d:", len);
			print_n(&apft->elem[i + 1], len);
			tree_printer(0, "\n");
			i += len;
			break;
		}
	}
}
