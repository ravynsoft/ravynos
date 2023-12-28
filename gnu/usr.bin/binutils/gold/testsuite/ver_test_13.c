__asm__ (".symver foo, foo@VER_0");

int foo(void);

int foo(void) {
  return 0;
}
