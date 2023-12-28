#include "script_test_11.h"

static unsigned int buffer1[256] __attribute((used));
static unsigned int buffer2[256] __attribute((used)) = { 1 };

unsigned int foo __attribute__((section(".foo")));
extern char __foo_start;
extern char __foo_end;

int
main (void)
{
  if (&__foo_end - &__foo_start != sizeof(foo))
    return 1;
  if (!ptr_equal(&__foo_start, (char *)&foo))
    return 2;
  return 0;
}
