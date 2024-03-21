int
library_func1 (void)
{
  return 2;
}

int global = 1;

#ifdef WITH_IFUNC

static int minus_one (void) { return -1; }
static int zero (void) { return 0; }

void * library_func2_ifunc (void) __asm__ ("library_func2");
void * library_func2_ifunc (void) { return global ? minus_one : zero ; }
__asm__(".type library_func2, %gnu_indirect_function");

extern int library_func2 (int);
extern __typeof (library_func2) library_func2 __asm__ ("__GI_library_func2");

__asm__(".global __GI_library_func2");
__asm__(".hidden __GI_library_func2");
__asm__("__GI_library_func2 = library_func2");

int
library_func (int x)
{
  return library_func2 (x);
}

#else /* WITHOUT_IFUNC */

int
library_func2 (void)
{
  return 3;
}

#endif
