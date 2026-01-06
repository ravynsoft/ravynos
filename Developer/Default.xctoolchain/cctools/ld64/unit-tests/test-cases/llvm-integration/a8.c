
static signed int i = 0;
extern int foo1(void);
extern void foo2(void);

void foo2(void) {

  i = -1;

}

static int foo3() {
  return 10;
}

int foo1(void)
{
  int data = 0;
  if (i < 0)
    data = foo3();
  data += 42;
  return data;
}
