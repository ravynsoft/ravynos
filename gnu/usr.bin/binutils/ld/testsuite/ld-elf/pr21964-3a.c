extern int __start___verbose[];
extern int __stop___verbose[];
int
foo3 (void)
{
  if (& __start___verbose[0] == & __stop___verbose[0]
      || __start___verbose[0] != 6)
    return -1;
  else
    return 0;
}
