#include "posix.h"

#include <stdint.h>
#include <string.h>

#include "apf.h"
#include "args.h"

struct apf_arg
apf_tag_int32_t(int32_t val)
{
	return (struct apf_arg){ .i32 = val, .tag = apfat_int32_t };
}

struct apf_arg
apf_tag_str(const char *val)
{
	return (struct apf_arg){ .str = val, .tag = apfat_str };
}

struct apf_arg
apf_tag_float(float val)
{
	return (struct apf_arg){ .flt = val, .tag = apfat_float };
}
