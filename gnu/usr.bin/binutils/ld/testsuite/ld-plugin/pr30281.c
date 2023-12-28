extern __inline __attribute__((__gnu_inline__)) void foo(void) {}
__attribute__((__symver__("foo@GLIBC_2.2.5")))
int __collector_foo_2_2(void) {}
void foo(void) {}
