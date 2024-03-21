extern int __attribute__ ((weak)) fun (void);
int
bar (void)
{
  if (&fun != 0)
    return fun ();
  return 0;
}
