int common1[8];
void
foo ()
{
  int i;
  for (i = 0; i < sizeof (common1)/ sizeof (common1[0]); i++)
    common1[i] = -1;
}
