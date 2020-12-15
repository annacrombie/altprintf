#ifndef APF_H
#define APF_H

#include <stdint.h>

enum apf_type {
	apft_raw = 0,
	apft_dat = 1,
	apft_id_num = 0,
	apft_id_sym = 1,
	apft_id_lit = 2,
};

enum apf_dat_flags {
	apff_align         = 1 << 7,
	apff_align_l       = 0 << 7,
	apff_align_r       = 1 << 7,
	apff_align_chr     = 1 << 6,
	apff_width         = 1 << 5,
	apff_prec          = 1 << 4,
	apff_trans         = 1 << 3,
	apff_unused_1      = 1 << 2,
	apff_conditional   = 1 << 1,
	apff_max_width     = 255,
};

enum apf_transform {
	apf_trans_binary,
	apf_trans_hex,
	apf_trans_none = 255
};

struct apf_template {
	uint16_t len;
	uint8_t *elem;
};
#endif
