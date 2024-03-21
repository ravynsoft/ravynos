extern void abort (void);
extern void foo (void);
extern void *bar (void);

typedef void (*func_p) (void);

extern const func_p p1;

func_p p2 = &foo;
func_p p3 = &foo;

int
main ()
{
  void *p = bar ();
  p1 ();
  p2 ();
  p3 ();
  if (p != p1)
    abort ();
  return 0;
}
