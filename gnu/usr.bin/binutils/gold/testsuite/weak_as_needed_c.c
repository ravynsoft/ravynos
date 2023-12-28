#include <stdio.h>

__asm__ (".symver bar_v1, bar@v1");
__asm__ (".symver bar_v2, bar@@v2");

void bar_v1(void);
void bar_v2(void);
void baz(void);
void t4(void);

void bar_v1(void)
{
  printf("weak_as_needed_c: bar_v1\n");
}

void bar_v2(void)
{
  printf("weak_as_needed_c: bar_v2\n");
}

void baz(void)
{
  printf("weak_as_needed_c: baz\n");
}

void t4(void)
{
  printf("weak_as_needed_c: t4\n");
}
