extern int bar(void);
extern int i;

void foo(void)
{
  i = bar();
}
