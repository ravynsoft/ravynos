/* generate_uudmap.c:

   Create three .h files, whose names are specified as argv[1..3],
   but are usually uudmap.h, bitcount.h and mg_data.h.

   It uses mg_raw.h as input, plus it relies on the C compiler knowing
   the ord value of character literals under EBCDIC, to generate output
   tables on an order which are platform-specific.

   The outputs are:

     uudmap.h:
         The values which will populate PL_uumap[], as used by
         unpack('u').

     bitcount.h
          The values which will populate PL_bitcount[]:
          this is a count of bits for each U8 value 0..255.
          (I'm not sure why this has to be generated - surely it's
          platform-independent - DAPM.)

     mg_data.h
          Takes the input from mg_raw.h and sorts by it magic char;
          the values will populate PL_magic_data[]: this is an array of
          per-magic U8 values containing an index into PL_magic_vtables[]
          plus two flags:
             PERL_MAGIC_READONLY_ACCEPTABLE
             PERL_MAGIC_VALUE_MAGIC

   Originally this program just generated uudmap.h
   However, when we later wanted to generate bitcount.h, it was easier to
   refactor it and keep the same name, than either alternative - rename it,
   or duplicate all of the Makefile logic for a second program.
   Ditto when mg_data.h was added.
*/

#include <stdio.h>
#include <stdlib.h>
/* If it turns out that we need to make this conditional on config.sh derived
   values, it might be easier just to rip out the use of strerrer().  */
#include <string.h>
/* If a platform doesn't support errno.h, it's probably so strange that
   "hello world" won't port easily to it.  */
#include <errno.h>

struct mg_data_raw_t {
    unsigned char type;
    const char *value;
    const char *comment;
};

static struct mg_data_raw_t mg_data_raw[] = {
#ifdef WIN32
#  include "..\mg_raw.h"
#else
#  include "mg_raw.h"
#endif
    {0, 0, 0}
};

struct mg_data_t {
    const char *value;
    const char *comment;
};

static struct mg_data_t mg_data[256];

static void
format_mg_data(FILE *out, const void *thing, size_t count) {
  const struct mg_data_t *p = (const struct mg_data_t *)thing;

  while (1) {
      if (p->value) {
          fprintf(out, "    %s\n    %s", p->comment, p->value);
      } else {
          fputs("    0", out);
      }
      ++p;
      if (!--count)
          break;
      fputs(",\n", out);
  }
  fputc('\n', out);
}

static void
format_char_block(FILE *out, const void *thing, size_t count) {
  const char *block = (const char *)thing;

  fputs("    ", out);
  while (count--) {
    fprintf(out, "%d", *block);
    block++;
    if (count) {
      fputs(", ", out);
      if (!(count & 15)) {
        fputs("\n    ", out);
      }
    }
  }
  fputc('\n', out);
}

static void
output_to_file(const char *progname, const char *filename,
               void (format_function)(FILE *out, const void *thing, size_t count),
               const void *thing, size_t count,
               const char *header
) {
  FILE *const out = fopen(filename, "w");

  if (!out) {
    fprintf(stderr, "%s: Could not open '%s': %s\n", progname, filename,
            strerror(errno));
    exit(1);
  }

  fprintf(out, "/* %s:\n", filename);
  fprintf(out, " * THIS FILE IS AUTO-GENERATED DURING THE BUILD by: %s\n",
                progname);
  fprintf(out, " *\n%s\n*/\n{\n", header);
  format_function(out, thing, count);
  fputs("}\n", out);

  if (fclose(out)) {
    fprintf(stderr, "%s: Could not close '%s': %s\n", progname, filename,
            strerror(errno));
    exit(1);
  }
}


static const char PL_uuemap[]
= "`!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

typedef unsigned char U8;

/* This will ensure it is initialized to all zeros.  */
static char PL_uudmap[256];
static char PL_bitcount[256];

int main(int argc, char **argv) {
  size_t i;
  int bits;
  struct mg_data_raw_t *p = mg_data_raw;

  if (argc < 4 || argv[1][0] == '\0' || argv[2][0] == '\0'
      || argv[3][0] == '\0') {
    fprintf(stderr, "Usage: %s uudemap.h bitcount.h mg_data.h\n", argv[0]);
    return 1;
  }

  for (i = 0; i < sizeof(PL_uuemap) - 1; ++i)
    PL_uudmap[(U8)PL_uuemap[i]] = (char)i;
  /*
   * Because ' ' and '`' map to the same value,
   * we need to decode them both the same.
   */
  PL_uudmap[(U8)' '] = 0;

  output_to_file(argv[0], argv[1], &format_char_block,
                 (const void *)PL_uudmap, sizeof(PL_uudmap),
        " * These values will populate PL_uumap[], as used by unpack('u')"
  );

  for (bits = 1; bits < 256; bits++) {
    if (bits & 1)	PL_bitcount[bits]++;
    if (bits & 2)	PL_bitcount[bits]++;
    if (bits & 4)	PL_bitcount[bits]++;
    if (bits & 8)	PL_bitcount[bits]++;
    if (bits & 16)	PL_bitcount[bits]++;
    if (bits & 32)	PL_bitcount[bits]++;
    if (bits & 64)	PL_bitcount[bits]++;
    if (bits & 128)	PL_bitcount[bits]++;
  }

  output_to_file(argv[0], argv[2], &format_char_block,
                 (const void *)PL_bitcount, sizeof(PL_bitcount),
     " * These values will populate PL_bitcount[]:\n"
     " * this is a count of bits for each U8 value 0..255"
  );

  while (p->value) {
      mg_data[p->type].value = p->value;
      mg_data[p->type].comment = p->comment;
      ++p;
  }
      
  output_to_file(argv[0], argv[3], &format_mg_data,
                 (const void *)mg_data, sizeof(mg_data)/sizeof(mg_data[0]),
     " * These values will populate PL_magic_data[]: this is an array of\n"
     " * per-magic U8 values containing an index into PL_magic_vtables[]\n"
     " * plus two flags:\n"
     " *    PERL_MAGIC_READONLY_ACCEPTABLE\n"
     " *    PERL_MAGIC_VALUE_MAGIC"
  );

  return 0;
}
