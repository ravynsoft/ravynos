extern int bar(void) __attribute__((__visibility__("hidden"), __const__));
extern void baz(int);

void foo(char c)
{
  int i;

  if (bar())
    i = c;
  else
    i = c;

  baz(i);
}
