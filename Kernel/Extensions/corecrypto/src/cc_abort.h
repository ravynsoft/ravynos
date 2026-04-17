#include <corecrypto/cc_config.h>

#if CC_KERNEL
#include <kern/debug.h>
#define cc_abort panic
#else
#include <stdio.h>
#include <stdlib.h>
#define cc_abort(msg, ...) do { fprintf(stderr, msg, __VA_ARGS__); abort(); } while (0)
#endif
