extern int foo __attribute ((weak));

int
_start (void)
{
  if (&foo)
    return foo;
  return 0;
}
