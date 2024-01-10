#ifndef NASM_WARNINGS_H
#define NASM_WARNINGS_H

#ifndef WARN_SHR
# error "warnings.h should only be included from within error.h"
#endif

enum warn_index {
	WARN_IDX_NONE                    =   0, /* not suppressible */
	WARN_IDX_DB_EMPTY                =   1, /* no operand for data declaration */
	WARN_IDX_EA_ABSOLUTE             =   2, /* absolute address cannot be RIP-relative */
	WARN_IDX_EA_DISPSIZE             =   3, /* displacement size ignored on absolute address */
	WARN_IDX_FLOAT_DENORM            =   4, /* floating point denormal */
	WARN_IDX_FLOAT_OVERFLOW          =   5, /* floating point overflow */
	WARN_IDX_FLOAT_TOOLONG           =   6, /* too many digits in floating-point number */
	WARN_IDX_FLOAT_UNDERFLOW         =   7, /* floating point underflow */
	WARN_IDX_FORWARD                 =   8, /* forward reference may have unpredictable results */
	WARN_IDX_LABEL_ORPHAN            =   9, /* labels alone on lines without trailing `:' */
	WARN_IDX_LABEL_REDEF             =  10, /* label redefined to an identical value */
	WARN_IDX_LABEL_REDEF_LATE        =  11, /* label (re)defined during code generation */
	WARN_IDX_NUMBER_OVERFLOW         =  12, /* numeric constant does not fit */
	WARN_IDX_OBSOLETE_NOP            =  13, /* instruction obsolete and is a noop on the target CPU */
	WARN_IDX_OBSOLETE_REMOVED        =  14, /* instruction obsolete and removed on the target CPU */
	WARN_IDX_OBSOLETE_VALID          =  15, /* instruction obsolete but valid on the target CPU */
	WARN_IDX_PHASE                   =  16, /* phase error during stabilization */
	WARN_IDX_PP_ELSE_ELIF            =  17, /* %elif after %else */
	WARN_IDX_PP_ELSE_ELSE            =  18, /* %else after %else */
	WARN_IDX_PP_EMPTY_BRACES         =  19, /* empty %{} construct */
	WARN_IDX_PP_ENVIRONMENT          =  20, /* nonexistent environment variable */
	WARN_IDX_PP_MACRO_DEF_CASE_SINGLE =  21, /* single-line macro defined both case sensitive and insensitive */
	WARN_IDX_PP_MACRO_DEF_GREEDY_SINGLE =  22, /* single-line macro */
	WARN_IDX_PP_MACRO_DEF_PARAM_SINGLE =  23, /* single-line macro defined with and without parameters */
	WARN_IDX_PP_MACRO_DEFAULTS       =  24, /* macros with more default than optional parameters */
	WARN_IDX_PP_MACRO_PARAMS_LEGACY  =  25, /* improperly calling multi-line macro for legacy support */
	WARN_IDX_PP_MACRO_PARAMS_MULTI   =  26, /* multi-line macro calls with wrong parameter count */
	WARN_IDX_PP_MACRO_PARAMS_SINGLE  =  27, /* single-line macro calls with wrong parameter count */
	WARN_IDX_PP_MACRO_REDEF_MULTI    =  28, /* redefining multi-line macro */
	WARN_IDX_PP_OPEN_BRACES          =  29, /* unterminated %{...} */
	WARN_IDX_PP_OPEN_BRACKETS        =  30, /* unterminated %[...] */
	WARN_IDX_PP_OPEN_STRING          =  31, /* unterminated string */
	WARN_IDX_PP_REP_NEGATIVE         =  32, /* regative %rep count */
	WARN_IDX_PP_SEL_RANGE            =  33, /* %sel() argument out of range */
	WARN_IDX_PP_TRAILING             =  34, /* trailing garbage ignored */
	WARN_IDX_PRAGMA_BAD              =  35, /* malformed %pragma */
	WARN_IDX_PRAGMA_EMPTY            =  36, /* empty %pragma directive */
	WARN_IDX_PRAGMA_NA               =  37, /* %pragma not applicable to this compilation */
	WARN_IDX_PRAGMA_UNKNOWN          =  38, /* unknown %pragma facility or directive */
	WARN_IDX_PREFIX_BND              =  39, /* invalid BND prefix */
	WARN_IDX_PREFIX_HLE              =  40, /* invalid HLE prefix */
	WARN_IDX_PREFIX_LOCK             =  41, /* LOCK prefix on unlockable instructions */
	WARN_IDX_PREFIX_OPSIZE           =  42, /* invalid operand size prefix */
	WARN_IDX_PREFIX_SEG              =  43, /* segment prefix ignored in 64-bit mode */
	WARN_IDX_PTR                     =  44, /* non-NASM keyword used in other assemblers */
	WARN_IDX_REGSIZE                 =  45, /* register size specification ignored */
	WARN_IDX_UNKNOWN_WARNING         =  46, /* unknown warning in -W/-w or warning directive */
	WARN_IDX_USER                    =  47, /* %warning directives */
	WARN_IDX_WARN_STACK_EMPTY        =  48, /* warning stack empty */
	WARN_IDX_ZEROING                 =  49, /* RESx in initialized section becomes zero */
	WARN_IDX_ZEXT_RELOC              =  50, /* relocation zero-extended to match output format */
	WARN_IDX_OTHER                   =  51, /* any warning not specifically mentioned above */
	WARN_IDX_ALL                     =  52  /* all possible warnings */
};

enum warn_const {
	WARN_NONE                        =   0 << WARN_SHR,
	WARN_DB_EMPTY                    =   1 << WARN_SHR,
	WARN_EA_ABSOLUTE                 =   2 << WARN_SHR,
	WARN_EA_DISPSIZE                 =   3 << WARN_SHR,
	WARN_FLOAT_DENORM                =   4 << WARN_SHR,
	WARN_FLOAT_OVERFLOW              =   5 << WARN_SHR,
	WARN_FLOAT_TOOLONG               =   6 << WARN_SHR,
	WARN_FLOAT_UNDERFLOW             =   7 << WARN_SHR,
	WARN_FORWARD                     =   8 << WARN_SHR,
	WARN_LABEL_ORPHAN                =   9 << WARN_SHR,
	WARN_LABEL_REDEF                 =  10 << WARN_SHR,
	WARN_LABEL_REDEF_LATE            =  11 << WARN_SHR,
	WARN_NUMBER_OVERFLOW             =  12 << WARN_SHR,
	WARN_OBSOLETE_NOP                =  13 << WARN_SHR,
	WARN_OBSOLETE_REMOVED            =  14 << WARN_SHR,
	WARN_OBSOLETE_VALID              =  15 << WARN_SHR,
	WARN_PHASE                       =  16 << WARN_SHR,
	WARN_PP_ELSE_ELIF                =  17 << WARN_SHR,
	WARN_PP_ELSE_ELSE                =  18 << WARN_SHR,
	WARN_PP_EMPTY_BRACES             =  19 << WARN_SHR,
	WARN_PP_ENVIRONMENT              =  20 << WARN_SHR,
	WARN_PP_MACRO_DEF_CASE_SINGLE    =  21 << WARN_SHR,
	WARN_PP_MACRO_DEF_GREEDY_SINGLE  =  22 << WARN_SHR,
	WARN_PP_MACRO_DEF_PARAM_SINGLE   =  23 << WARN_SHR,
	WARN_PP_MACRO_DEFAULTS           =  24 << WARN_SHR,
	WARN_PP_MACRO_PARAMS_LEGACY      =  25 << WARN_SHR,
	WARN_PP_MACRO_PARAMS_MULTI       =  26 << WARN_SHR,
	WARN_PP_MACRO_PARAMS_SINGLE      =  27 << WARN_SHR,
	WARN_PP_MACRO_REDEF_MULTI        =  28 << WARN_SHR,
	WARN_PP_OPEN_BRACES              =  29 << WARN_SHR,
	WARN_PP_OPEN_BRACKETS            =  30 << WARN_SHR,
	WARN_PP_OPEN_STRING              =  31 << WARN_SHR,
	WARN_PP_REP_NEGATIVE             =  32 << WARN_SHR,
	WARN_PP_SEL_RANGE                =  33 << WARN_SHR,
	WARN_PP_TRAILING                 =  34 << WARN_SHR,
	WARN_PRAGMA_BAD                  =  35 << WARN_SHR,
	WARN_PRAGMA_EMPTY                =  36 << WARN_SHR,
	WARN_PRAGMA_NA                   =  37 << WARN_SHR,
	WARN_PRAGMA_UNKNOWN              =  38 << WARN_SHR,
	WARN_PREFIX_BND                  =  39 << WARN_SHR,
	WARN_PREFIX_HLE                  =  40 << WARN_SHR,
	WARN_PREFIX_LOCK                 =  41 << WARN_SHR,
	WARN_PREFIX_OPSIZE               =  42 << WARN_SHR,
	WARN_PREFIX_SEG                  =  43 << WARN_SHR,
	WARN_PTR                         =  44 << WARN_SHR,
	WARN_REGSIZE                     =  45 << WARN_SHR,
	WARN_UNKNOWN_WARNING             =  46 << WARN_SHR,
	WARN_USER                        =  47 << WARN_SHR,
	WARN_WARN_STACK_EMPTY            =  48 << WARN_SHR,
	WARN_ZEROING                     =  49 << WARN_SHR,
	WARN_ZEXT_RELOC                  =  50 << WARN_SHR,
	WARN_OTHER                       =  51 << WARN_SHR
};

struct warning_alias {
	const char *name;
	enum warn_index warning;
};

#define NUM_WARNING_ALIAS 68
extern const char * const warning_name[53];
extern const char * const warning_help[53];
extern const struct warning_alias warning_alias[NUM_WARNING_ALIAS];
extern const uint8_t warning_default[52];
extern uint8_t warning_state[52];

#endif /* NASM_WARNINGS_H */
