extern __thread char bar[];
extern char size_of_bar __asm__ ("bar@SIZE");
char *bar_size = &size_of_bar;

char *
get_bar (int i, int v)
{
  bar[i] = v;
  return bar;
}
