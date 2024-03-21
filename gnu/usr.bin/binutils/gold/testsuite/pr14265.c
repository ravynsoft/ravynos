int foo0 __attribute__((used,section(".foo0.0")));
int foo1 __attribute__((used,section(".foo1.0")));
int foo2 __attribute__((used,section(".foo2.0")));

extern unsigned long    __foo0_start;
extern unsigned long    __foo0_end;

extern unsigned long    __foo1_start;
extern unsigned long    __foo1_end;

extern unsigned long    __foo2_start;
extern unsigned long    __foo2_end;

int
main (void)
{
  return ((__foo0_end - __foo0_start) -
	  (__foo1_end - __foo1_start) -
	  (__foo2_end - __foo2_start));
}
