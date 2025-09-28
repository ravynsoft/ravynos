extern int bar(void);
typedef int (*func_p) (void);

func_p
get_bar (void)
{
  return bar;
}
