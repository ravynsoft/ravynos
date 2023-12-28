void
foo (void)
{
}

__asm__ (".symver foo,foo@FOO");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__ (".symver .foo,.foo@FOO");
#endif
