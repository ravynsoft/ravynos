extern int counter;

extern void g(void);

void f(void)
{
  g();
  counter++;
}
