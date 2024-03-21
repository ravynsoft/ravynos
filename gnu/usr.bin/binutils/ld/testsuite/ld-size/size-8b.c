__thread char bar[10];

void
set_bar (int i, int v)
{
  bar[i] = v;
}
