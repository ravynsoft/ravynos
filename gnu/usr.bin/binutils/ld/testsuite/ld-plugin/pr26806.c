#include <unistd.h>
int foo (int x) { if (__builtin_constant_p (x)) return getpid (); return 0; }
