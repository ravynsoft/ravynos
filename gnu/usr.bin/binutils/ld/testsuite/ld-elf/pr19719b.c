extern int __attribute__ ((weak)) fun (void);
int
foo (void)
{
  if (&fun != 0)
    return fun ();
  return 0;
}
