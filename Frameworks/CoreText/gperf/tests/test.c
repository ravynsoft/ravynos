/*
   Tests the generated perfect hash function.
   The -v option prints diagnostics as to whether a word is in
   the set or not.  Without -v the program is useful for timing.
*/

#include <stdio.h>
#include <string.h>

extern const char * in_word_set (const char *, size_t);

#define MAX_LEN 80

int
main (int argc, char *argv[])
{
  int  verbose = argc > 1 ? 1 : 0;
  char buf[MAX_LEN];

  while (fgets (buf, MAX_LEN, stdin))
    {
      if (strlen (buf) > 0 && buf[strlen (buf) - 1] == '\n')
        buf[strlen (buf) - 1] = '\0';

      if (in_word_set (buf, strlen (buf)))
        {
          if (verbose)
            printf ("in word set %s\n", buf);
        }
      else
        {
          if (verbose)
            printf ("NOT in word set %s\n", buf);
        }
    }

  return 0;
}
