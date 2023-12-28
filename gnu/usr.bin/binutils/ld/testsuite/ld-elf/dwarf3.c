/* This test is actually used to test for a segfault that came from the bfd
   dwarf parsing code in the case when there is _no_ dwarf info.  */

extern void bar (int a);

int
main ()
{
  bar (1);
  bar (2);

  return 0;
}
