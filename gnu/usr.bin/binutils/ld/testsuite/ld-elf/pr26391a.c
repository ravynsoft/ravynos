typedef void (*func_p) (void);

extern func_p bar1_p (void);
extern func_p bar2_p (void);
extern func_p bar3_p (void);

int
main ()
{
  func_p f;
  f = bar1_p ();
  f ();
  f = bar2_p ();
  f ();
  f = bar3_p ();
  f ();
  return 0;
}
