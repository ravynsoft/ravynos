TOTO = 4096;
TOTO += 4096;

SECTIONS
{
  .text TOTO :
  {
    x = ABSOLUTE(TOTO);
    *(.text)
  }
}
