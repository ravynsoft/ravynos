__thread int * foo;

void
bar (void)
{
  *foo = 1;
}
