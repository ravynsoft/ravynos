extern int __start___verbose[];
extern int __stop___verbose[];
int
foo1 (void)
{
  static int my_var __attribute__((used, section("__verbose"))) = 5;
  if (& __start___verbose[0] == & __stop___verbose[0]
      && __start___verbose[0] != 5)
    return -1;
  else
    return 0;
}
