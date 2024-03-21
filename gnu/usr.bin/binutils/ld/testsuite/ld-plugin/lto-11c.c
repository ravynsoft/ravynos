extern int printf(const char *, ...);

void bar(void)
{
  printf("Hello from %s!\n", __FUNCTION__);
}
