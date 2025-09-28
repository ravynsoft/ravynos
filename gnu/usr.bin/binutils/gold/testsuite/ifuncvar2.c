/* Test global variable initialized to hidden STT_GNU_IFUNC symbol.  */

extern void foo (void);
void (*f) (void) = &foo;

extern void bar (void);

void
bar (void)
{
  f ();
}
