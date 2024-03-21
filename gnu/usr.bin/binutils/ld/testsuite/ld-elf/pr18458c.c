extern void a(void);
extern void b(void);

void dummy (void)
{
  a();
}
int
compare (void (*f)(void))
{
  return a == f;
}
int
main (void)
{
  b ();
  return 0;
}
