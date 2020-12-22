#ifndef COMMON_H
#define COMMON_H

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

enum apf_template_flags {
	apftf_has_id_args = 1 << 3,
	apftf_has_sym_args = 1 << 4,
};

enum apf_transform {
	apf_trans_binary,
	apf_trans_hex,
	apf_trans_none = 255
};

enum apf_offset {
	apf_data_hdr = 3,
	apf_cond_hdr = 4,
	apf_cond_arm_1 = 0,
	apf_cond_arm_2 = 2,
};
#endif
