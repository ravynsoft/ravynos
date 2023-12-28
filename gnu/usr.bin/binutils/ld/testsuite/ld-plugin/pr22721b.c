__thread int foo_var = 1;

int
_start (void)
{
  return foo_var;
}
