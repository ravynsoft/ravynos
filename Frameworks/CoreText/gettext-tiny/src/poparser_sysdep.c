#include "poparser.h"

#define ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _R, ...) _R
#define COUNT_ARGS(...) ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define ENTRY(FMT, ...) { \
	.format = FMT, \
	.repl = __VA_ARGS__, \
	.cnt = COUNT_ARGS(__VA_ARGS__) \
	}

const sysdep_case_t sysdep_cases[] = {
#include "sysdep.h"
};

