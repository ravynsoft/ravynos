extern void foobar (void);
int
main (int argc, char **argv)
{
  if (__builtin_constant_p (argc))
    foobar ();
  return 0;
}
