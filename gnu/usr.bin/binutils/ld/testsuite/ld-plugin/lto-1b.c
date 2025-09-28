extern unsigned long long bar (unsigned long long);

int
main (int argc, char **argv)
{
  unsigned long long d = bar ((unsigned long long) (argc + 1));
  return d;
}
