static int seven = 7;
static int *__attribute__((section("auto"))) auto_10 = &seven;

int
eight (void)
{
  extern int *__start_auto[], *__stop_auto[];
  return *auto_10 + __stop_auto - __start_auto;
}
