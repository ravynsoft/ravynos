int unused_var = 7;
int used_var = 7;

int
unused_func (int v)
{
  return 3 * unused_var;
}

int
__attribute__((noinline))
used_func (int v)
{
  return 2 * used_var;
}

int
main (void)
{
  return used_func (5);
}

void
dummy_func (void)
{
  /* These are here in case the target prepends an underscore to
     the start of function names.  They are inside a dummy function
     so that they will appear at the end of gcc's assembler output,
     after the definitions of main() and used_func(), rather than
     at the beginning of the file.  */

  __asm__(".ifndef main\n\
.global main\n\
.set main, _main\n\
.endif");

  __asm__(".ifndef used_func\n\
.global used_func\n\
.set used_func, _used_func\n\
.endif");
}
