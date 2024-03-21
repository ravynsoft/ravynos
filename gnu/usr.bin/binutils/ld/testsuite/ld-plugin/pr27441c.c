extern int func1 (void);
extern int func2 (void);

int
callthem (void)
{
  return func1 () + func2 ();
}
