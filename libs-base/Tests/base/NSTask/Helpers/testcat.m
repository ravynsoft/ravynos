#include	<stdio.h>

/* Just cat the GNUmakefle
 */
int
main(int argc, char **argv)
{
  FILE	*f = fopen("GNUmakefile", "rb");
  int	c;

  while ((c = fgetc(f)) != EOF)
    putchar(c);
  fclose(f);
  fflush(stdout);
  return 0;
}

