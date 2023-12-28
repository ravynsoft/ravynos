static int foo __attribute__((__used__));
int bar;

/* This is sometimes linked as a main program, sometimes via ld -r, and
   sometimes via ld -shared.  */
int main (void)
{
  return 0;
}
