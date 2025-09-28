#include <stdint.h>

extern void bar (void);
extern void foo (void);
extern void foo_alias (void);
extern void check_ptr_eq (void (*) (void), void (*) (void));

#if defined(__GNUC__) && (__GNUC__ * 1000 + __GNUC_MINOR__) >= 4005
__attribute__ ((noinline, noclone))
#else
__attribute__ ((noinline))
#endif
int
foo_p (void)
{
  return (intptr_t) &foo == 0x12345678 ? 1 : 0;
}

int
main (void)
{
  foo ();
  foo_p ();
  bar ();
  check_ptr_eq (&foo, &foo_alias);
  return 0;
}
