extern int printf(const char *, ...);

void foo(void)
{
  printf("Hello from %s!\n", __FUNCTION__);
}
