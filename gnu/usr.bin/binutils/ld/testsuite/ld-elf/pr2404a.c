#include <time.h>
#include <sys/times.h>

int
bar (void)
{
  struct tms buf;
  clock_t ticks = times (&buf);
  return ticks == 0 && time (NULL) == 0;
}
