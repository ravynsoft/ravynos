extern int foo(void);
typedef int (*func_p) (void);
extern func_p foo_ptr;

void
check_foo (void)
{
  if (foo_ptr != foo)
    __builtin_abort ();
  if (foo_ptr() != 1)
    __builtin_abort ();
  if (foo() != 1)
    __builtin_abort ();
}
