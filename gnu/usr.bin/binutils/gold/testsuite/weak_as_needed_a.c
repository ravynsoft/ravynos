extern void bar(void) __attribute__ (( weak ));
extern void t4(void);

void foo(void);

void foo(void)
{
  bar();
  t4();
}
