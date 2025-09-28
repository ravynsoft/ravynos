#include <stdlib.h>
#include <string.h>

extern int *p_int_from_a_2;
extern const char *hello (void);

int main (void) {
  if (*p_int_from_a_2 != 0x11223344)
    abort ();
  if (strcmp(hello(), "Hello, world!") != 0)
    abort ();
  return 0;
}
