int (*p)(void);

int
main ()
{
  return p () != 0x1234;
}
