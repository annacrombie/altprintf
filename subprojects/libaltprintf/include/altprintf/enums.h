#ifndef _ALTPRINTF_ENUM_H
#define _ALTPRINTF_ENUM_H
enum arg_type { FMul, FTern, FAlign, FInt, FChar, FDouble, FString, FRaw, FNone,
		FEnd };
enum align { Left, Right, Center };

enum altprintf_err {
	apfe_none,
	apfe_invalid_token,
	apfe_missing_argument
};

extern enum altprintf_err apf_err;
#endif
