__thread int yyy = 100;
extern __thread int zzz;

int
foo (void)
{
  return zzz;
}
