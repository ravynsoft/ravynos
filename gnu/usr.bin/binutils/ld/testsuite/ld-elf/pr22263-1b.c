extern __thread int *foo;
 
static int x;

extern void bar (void);

int
_start ()
{
  foo = &x;
  return 0;
}
