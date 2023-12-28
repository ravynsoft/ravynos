int select (void) { return 2; }

extern int f1 (int);

int f2 (int x)
{
  if (x > 0)
    return x * f1 (x - 1);
  return 22222;
}
