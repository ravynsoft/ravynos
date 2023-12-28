#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
static const char __evoke_link_warning_foobar[]
__attribute__ ((used, section (".gnu.warning..foobar\n\t#"))) = "foobar";
#else
static const char __evoke_link_warning_foobar[]
__attribute__ ((used, section (".gnu.warning.foobar\n\t#"))) = "foobar";
#endif
