#ifdef __x86_64__
#include <architecture/i386/asm_help.h>
#else
#error Unsupported platform
#endif
 
.text

LEAF(___cxa_atexit, 0)
        xorq %rax, %rax
        ret
