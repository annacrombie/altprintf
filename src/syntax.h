#ifndef SYNTAX_H
#define SYNTAX_H

#define FS_START    '%'
#define FS_DOUBLE_SEP '.'

#define FS_ESC L'\\'
#define FS_ESC_NL 'n'
#define FS_ESC_ESC 'e'

#define FS_T_STRING 's'
#define FS_T_CHAR   'c'
#define FS_T_DOUBLE 'f'
#define FS_T_INT    'd'
#define FS_T_MUL    '*'
#define FS_T_TERN   '?'
#define FS_T_ALIGN  '='

#define FS_A_PARENARG_S '('
#define FS_A_PARENARG_E ')'
#define FS_A_ANGLEARG_S '<'
#define FS_A_ANGLEARG_E '>'

#define FS_A_CHARARG     '~'
#define FS_A_LALIGN      '-'
#define FS_A_CALIGN      '^'
#define FS_A_SPAD        ' '
#define FS_A_ZPAD        '0'
#define FS_A_PREC        '.'

#define FS_D_PREC   100

#endif
