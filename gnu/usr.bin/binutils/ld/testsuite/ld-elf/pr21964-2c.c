#include <dlfcn.h>
#include <stdio.h>

extern int foo1 (void);

int main()
{
  void *dl;
  void *sym;
  int (*func) (void);

  if (foo1 () != 0)
    return 1;

  dl = dlopen("pr21964-2b.so", RTLD_LAZY);
  if (!dl)
    return 2;

  sym = dlsym(dl, "__start___verbose");
  if (!sym)
    return 3;

  func = dlsym(dl, "foo2");
  if (!func)
    return 4;
  if (func () == 0)
    printf ("PASS\n");

  dlclose(dl);

  return 0;
}
