extern int foo4(void);

int foo4(void)
{
  return 21;
}
static int myfoo()
{
  return foo4();
}
int foo2() {
	return myfoo();
}
