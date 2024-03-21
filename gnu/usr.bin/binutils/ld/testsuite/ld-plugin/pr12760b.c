void bar (void) {}
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
static const char __warn_bar[]
__attribute__ ((used, section (".gnu.warning..bar"))) = "Bad bar";
#else
static const char __warn_bar[]
__attribute__ ((used, section (".gnu.warning.bar"))) = "Bad bar";
#endif
