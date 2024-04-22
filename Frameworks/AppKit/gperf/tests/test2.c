/*
   Tests the generated perfect hash function.
   The -v option prints diagnostics as to whether a word is in
   the set or not.  Without -v the program is useful for timing.
*/

#include <stdio.h>

/* Support for SET_BINARY. */
#include <fcntl.h>
#if !defined O_BINARY && defined _O_BINARY
# define O_BINARY _O_BINARY
#endif
#ifdef __BEOS__
# undef O_BINARY
#endif
#if O_BINARY
# include <io.h>
# define SET_BINARY(f) setmode (f, O_BINARY)
#else
# define SET_BINARY(f) (void)0
#endif

extern struct language * in_word_set (const char *, size_t);

#define MAX_LEN 80

int
main (int argc, char *argv[])
{
  int  verbose = argc > 1 ? 1 : 0;
  char buf[2*MAX_LEN];
  int buflen;

  /* We need to read stdin in binary mode. */
  SET_BINARY (0);

  for (;;)
    {
      /* Simulate gets(buf) with 2 bytes per character. */
      char *p = buf;
      while (fread (p, 2, 1, stdin) == 1)
        {
          if ((p[0] << 8) + p[1] == '\n')
            break;
          p += 2;
        }
      buflen = p - buf;

      if (buflen == 0)
        break;

      if (in_word_set (buf, buflen))
        {
          if (verbose)
            printf ("in word set:");
        }
      else
        {
          if (verbose)
            printf ("NOT in word set:");
        }

      for (p = buf; p < buf + buflen; p += 2)
        printf (" %02X%02X", (unsigned char) p[0], (unsigned char) p[1]);
      printf("\n");
    }

  return 0;
}
