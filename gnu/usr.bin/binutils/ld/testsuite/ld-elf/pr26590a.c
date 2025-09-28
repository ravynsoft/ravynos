int select (void) { return 1; }

extern int f2 (int);

int f1 (int x)
{
  if (x > 0)
    return x * f2 (x - 1);
  return 1;
}
