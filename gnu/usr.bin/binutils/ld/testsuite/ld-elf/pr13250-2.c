extern int common1[8];

extern void foo ();

int
bar ()
{
  foo ();
  return common1[4];
}
