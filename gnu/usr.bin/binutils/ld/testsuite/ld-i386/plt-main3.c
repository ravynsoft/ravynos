extern int bar(void);
typedef int (*func_p) (void);
extern func_p get_bar (void);

void
check_bar (void)
{
  func_p bar_ptr = get_bar ();
  if (bar_ptr != bar)
    __builtin_abort ();
  if (bar_ptr() != -1)
    __builtin_abort ();
  if (bar() != -1)
    __builtin_abort ();
}
