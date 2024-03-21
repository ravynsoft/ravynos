#include <stdio.h>
const char* sd_get_seats(void);

int
main (int argc, char **argv)
{
  printf("%s\n", sd_get_seats());
  return 0;
}
