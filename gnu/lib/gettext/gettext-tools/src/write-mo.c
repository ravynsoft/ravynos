/* Writing binary .mo files.
   Copyright (C) 1995-1998, 2000-2007, 2016, 2020, 2023 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, April 1995.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <alloca.h>

/* Specification.  */
#include "write-mo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

/* These two include files describe the binary .mo format.  */
#include "gmo.h"
#include "hash-string.h"

#include "byteswap.h"
#include "error.h"
#include "mem-hash-map.h"
#include "message.h"
#include "format.h"
#include "xsize.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "po-charset.h"
#include "msgl-iconv.h"
#include "msgl-header.h"
#include "binary-io.h"
#include "supersede.h"
#include "fwriteerror.h"
#include "gettext.h"

#define _(str) gettext (str)

#define freea(p) /* nothing */

/* Usually defined in <sys/param.h>.  */
#ifndef roundup
# if defined __GNUC__ && __GNUC__ >= 2
#  define roundup(x, y) ({typeof(x) _x = (x); typeof(y) _y = (y); \
                          ((_x + _y - 1) / _y) * _y; })
# else
#  define roundup(x, y) ((((x)+((y)-1))/(y))*(y))
# endif /* GNU CC2  */
#endif /* roundup  */

#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))


/* True if no conversion to UTF-8 is desired.  */
bool no_convert_to_utf8;

/* True if the redundant storage of instantiations of system-dependent strings
   shall be avoided.  */
bool no_redundancy;

/* Alignment of strings in resulting .mo file.  */
size_t alignment;

/* True if writing a .mo file in opposite endianness than the host.  */
bool byteswap;

/* True if no hash table in .mo is wanted.  */
bool no_hash_table;


/* Destructively changes the byte order of a 32-bit value in memory.  */
#define BSWAP32(x) (x) = bswap_32 (x)


/* Indices into the strings contained in 'struct pre_message' and
   'struct pre_sysdep_message'.  */
enum
{
  M_ID = 0,     /* msgid - the original string */
  M_STR = 1     /* msgstr - the translated string */
};

/* An intermediate data structure representing a 'struct string_desc'.  */
struct pre_string
{
  size_t length;
  const char *pointer;
};

/* An intermediate data structure representing a message.  */
struct pre_message
{
  struct pre_string str[2];
  const char *id_plural;
  size_t id_plural_len;
};

static int
compare_id (const void *pval1, const void *pval2)
{
  return strcmp (((const struct pre_message *) pval1)->str[M_ID].pointer,
                 ((const struct pre_message *) pval2)->str[M_ID].pointer);
}


/* An intermediate data structure representing a 'struct sysdep_segment'.  */
struct pre_sysdep_segment
{
  size_t length;
  const char *pointer;
};

/* An intermediate data structure representing a 'struct segment_pair'.  */
struct pre_segment_pair
{
  size_t segsize;
  const char *segptr;
  size_t sysdepref;
};

/* An intermediate data structure representing a 'struct sysdep_string'.  */
struct pre_sysdep_string
{
  unsigned int segmentcount;
  struct pre_segment_pair segments[1];
};

/* An intermediate data structure representing a message with system dependent
   strings.  */
struct pre_sysdep_message
{
  struct pre_sysdep_string *str[2];
  const char *id_plural;
  size_t id_plural_len;
};


/* Instantiating system dependent strings.
   This is a technique to make messages with system dependent strings work with
   musl libc's gettext() implementation, even though this implementation does
   not process the system dependent strings.  Namely, we store the actual
   runtime expansion of the string for this platform — we call this an
   "instantiation" of the string — in the table of static string pairs.
   This is redundant, but allows the same MO files to be used on musl libc
   (without GNU libintl) as on other platforms (with GNU libc or with GNU
   libintl).

   A survey of the PO files on translationproject.org shows that
     * Less than 9% of the messages of any PO file are system dependent strings.
       Therefore the increase of the size of the MO file is small.
     * None of these PO files uses the 'I' format string flag.

   There are few possible <inttypes.h> flavours.  Each such flavour gives rise
   to an instantation rule.  We ran this test program on various platforms:
   =============================================================================
   #include <inttypes.h>
   #include <stdio.h>
   #include <string.h>
   int main ()
   {
     printf ("%s\n", PRIuMAX);
     printf ("%s\n", PRIdMAX);
     printf ("%s  %s  %s  %s\n", PRIu8, PRIu16, PRIu32, PRIu64);
     printf ("%s  %s  %s  %s\n", PRId8, PRId16, PRId32, PRId64);
     printf ("%s  %s  %s  %s\n", PRIuLEAST8, PRIuLEAST16, PRIuLEAST32, PRIuLEAST64);
     printf ("%s  %s  %s  %s\n", PRIdLEAST8, PRIdLEAST16, PRIdLEAST32, PRIdLEAST64);
     printf ("%s  %s  %s  %s\n", PRIuFAST8, PRIuFAST16, PRIuFAST32, PRIuFAST64);
     printf ("%s  %s  %s  %s\n", PRIdFAST8, PRIdFAST16, PRIdFAST32, PRIdFAST64);
     printf ("%s\n", PRIuPTR);
     printf ("%s\n", PRIdPTR);
     printf ("Summary:\n");
     printf ("  MAX 8               LEAST8          FAST8           PTR\n");
     printf ("  |   |   |   |   |   |   |   |   |   |   |   |   |   |\n");
     printf ("| %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s %-3.*s |\n",
             (int) strlen (PRIuMAX) - 1, PRIuMAX,
             (int) strlen (PRIu8) - 1, PRIu8,
             (int) strlen (PRIu16) - 1, PRIu16,
             (int) strlen (PRIu32) - 1, PRIu32,
             (int) strlen (PRIu64) - 1, PRIu64,
             (int) strlen (PRIuLEAST8) - 1, PRIuLEAST8,
             (int) strlen (PRIuLEAST16) - 1, PRIuLEAST16,
             (int) strlen (PRIuLEAST32) - 1, PRIuLEAST32,
             (int) strlen (PRIuLEAST64) - 1, PRIuLEAST64,
             (int) strlen (PRIuFAST8) - 1, PRIuFAST8,
             (int) strlen (PRIuFAST16) - 1, PRIuFAST16,
             (int) strlen (PRIuFAST32) - 1, PRIuFAST32,
             (int) strlen (PRIuFAST64) - 1, PRIuFAST64,
             (int) strlen (PRIuPTR) - 1, PRIuPTR);
     return 0;
   }
   =============================================================================
   and found the following table.

   <inttypes.h>   MAX 8               LEAST8          FAST8           PTR
     flavour      |   |   |   |   |   |   |   |   |   |   |   |   |   |
   ------------ -----------------------------------------------------------
        0       | ll              ll              ll              ll      |
        1       | l               l               l               l   l   |
        2       | ll              ll              ll              ll  l   |
        3       | ll  hh  h       ll  hh  h       ll  hh  h       ll  l   |
        4       | ll  hh  h       ll  hh  h       ll  hh          ll      |
        5       | ll  hh  h       ll  hh  h       ll  hh          ll  ll  |
        6       | l               l               l       l   l   l   l   |
        7       | l   hh  h       l   hh  h       l   hh  h       l   l   |
        8       | l   hh  h       l   hh  h       l   hh          l   l   |
        9       | l   hh  h       l   hh  h       l   hh  l   l   l   l   |
       10       | j   hh  h       ll  hh  h       ll  hh  h       ll  l   |
       11       | j               ll              ll              ll      |
       12       | j               l               l               l   l   |
       13       | j               ll              ll              ll  l   |
       14       | I64             I64             I64             I64     |
       15       | I64             I64             I64             I64 I64 |

   Which <inttypes.h> flavour for which platforms?

   <inttypes.h>
     flavour     Platforms
   ------------  ---------------------------------------------------------------
        0        glibc 32-bit, musl 32-bit, NetBSD 32-bit
        1        musl 64-bit, NetBSD 64-bit, Haiku 64-bit
        2        Haiku 32-bit
        3        AIX 32-bit
        4        Solaris 32-bit, Cygwin 32-bit, MSVC 32-bit
        5        MSVC 64-bit
        6        glibc 64-bit
        7        AIX 64-bit
        8        Solaris 64-bit
        9        Cygwin 64-bit
       10        macOS 32-bit and 64-bit
       11        FreeBSD 32-bit, Android 32-bit
       12        FreeBSD 64-bit
       13        OpenBSD 32-bit and 64-bit
       14        mingw 32-bit
       15        mingw 64-bit
 */
struct sysdep_instantiation_rule
{
  const char *prefix_for_MAX;
  const char *prefix_for_8;      /* also for LEAST8 and FAST8 */
  const char *prefix_for_16;     /* also for LEAST16 */
  const char *prefix_for_64;     /* also for LEAST64 and FAST64 */
  const char *prefix_for_FAST16;
  const char *prefix_for_FAST32;
  const char *prefix_for_PTR;
};
const struct sysdep_instantiation_rule useful_instantiation_rules[] =
{
  /*  0 */ { "ll",  "",   "",  "ll",  "",  "",  ""    },
  /*  1 */ { "l",   "",   "",  "l",   "",  "",  "l"   },
#if 0 /* These instantiation rules are not useful.  They would just be bloat.  */
  /*  2 */ { "ll",  "",   "",  "ll",  "",  "",  "l"   },
  /*  3 */ { "ll",  "hh", "h", "ll",  "h", "",  "l"   },
  /*  4 */ { "ll",  "hh", "h", "ll",  "",  "",  ""    },
  /*  5 */ { "ll",  "hh", "h", "ll",  "",  "",  "ll"  },
  /*  6 */ { "l",   "",   "",  "l",   "l", "l", "l"   },
  /*  7 */ { "l",   "hh", "h", "l",   "h", "",  "l"   },
  /*  8 */ { "l",   "hh", "h", "l",   "",  "",  "l"   },
  /*  9 */ { "l",   "hh", "h", "l",   "l", "l", "l"   },
  /* 10 */ { "j",   "hh", "h", "ll",  "h", "",  "l"   },
  /* 11 */ { "j",   "",   "",  "ll",  "",  "",  ""    },
  /* 12 */ { "j",   "",   "",  "l",   "",  "",  "l"   },
  /* 13 */ { "j",   "",   "",  "ll",  "",  "",  "l"   },
  /* 14 */ { "I64", "",   "",  "I64", "",  "",  ""    },
  /* 15 */ { "I64", "",   "",  "I64", "",  "",  "I64" },
#endif
};

/* Concatenate a prefix and a conversion specifier.  */
static const char *
concat_prefix_cs (const char *prefix, char conversion)
{
  char *result = XNMALLOC (strlen (prefix) + 2, char);
  {
    char *p = result;
    p = stpcpy (p, prefix);
    *p++ = conversion;
    *p = '\0';
  }
  return result;
}

/* Expand a system dependent string segment for a specific instantation.
   Return NULL if unsupported.  */
static const char *
get_sysdep_segment_value (struct pre_sysdep_segment segment,
                          const struct sysdep_instantiation_rule *instrule)
{
  const char *name = segment.pointer;
  size_t len = segment.length;

  /* Test for an ISO C 99 section 7.8.1 format string directive.
     Syntax:
     P R I { d | i | o | u | x | X }
     { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
  if (len >= 3 && name[0] == 'P' && name[1] == 'R' && name[2] == 'I')
    {
      if (len >= 4
          && (name[3] == 'd' || name[3] == 'i' || name[3] == 'o'
              || name[3] == 'u' || name[3] == 'x' || name[3] == 'X'))
        {
          if (len == 5 && name[4] == '8')
            return concat_prefix_cs (instrule->prefix_for_8, name[3]);
          if (len == 6 && name[4] == '1' && name[5] == '6')
            return concat_prefix_cs (instrule->prefix_for_16, name[3]);
          if (len == 6 && name[4] == '3' && name[5] == '2')
            return concat_prefix_cs ("", name[3]);
          if (len == 6 && name[4] == '6' && name[5] == '4')
            return concat_prefix_cs (instrule->prefix_for_64, name[3]);
          if (len >= 9 && name[4] == 'L' && name[5] == 'E' && name[6] == 'A'
              && name[7] == 'S' && name[8] == 'T')
            {
              if (len == 10 && name[9] == '8')
                return concat_prefix_cs (instrule->prefix_for_8, name[3]);
              if (len == 11 && name[9] == '1' && name[10] == '6')
                return concat_prefix_cs (instrule->prefix_for_16, name[3]);
              if (len == 11 && name[9] == '3' && name[10] == '2')
                return concat_prefix_cs ("", name[3]);
              if (len == 11 && name[9] == '6' && name[10] == '4')
                return concat_prefix_cs (instrule->prefix_for_64, name[3]);
            }
          if (len >= 8 && name[4] == 'F' && name[5] == 'A' && name[6] == 'S'
              && name[7] == 'T')
            {
              if (len == 9 && name[8] == '8')
                return concat_prefix_cs (instrule->prefix_for_8, name[3]);
              if (len == 10 && name[8] == '1' && name[9] == '6')
                return concat_prefix_cs (instrule->prefix_for_FAST16, name[3]);
              if (len == 10 && name[8] == '3' && name[9] == '2')
                return concat_prefix_cs (instrule->prefix_for_FAST32, name[3]);
              if (len == 10 && name[8] == '6' && name[9] == '4')
                return concat_prefix_cs (instrule->prefix_for_64, name[3]);
            }
          if (len == 7 && name[4] == 'M' && name[5] == 'A' && name[6] == 'X')
            return concat_prefix_cs (instrule->prefix_for_MAX, name[3]);
          if (len == 7 && name[4] == 'P' && name[5] == 'T' && name[6] == 'R')
            return concat_prefix_cs (instrule->prefix_for_PTR, name[3]);
        }
    }
  /* Note: We cannot support the 'I' format directive flag here.  Because
       - If we expand the 'I' to "I", the expansion will not work on non-glibc
         systems (whose *printf() functions don't understand this flag).
       - If we expand the 'I' to "", the expansion will override the expansion
         produced at run time (see loadmsgcat.c) and will not produce the
         locale-specific outdigits as expected.  */
  return NULL;
}


/* Write the message list to the given open file.  */
static void
write_table (FILE *output_file, message_list_ty *mlp)
{
  char **msgctid_arr;
  size_t nstrings;
  size_t msg_arr_allocated;
  struct pre_message *msg_arr;
  size_t n_sysdep_strings;
  struct pre_sysdep_message *sysdep_msg_arr;
  size_t n_sysdep_segments;
  struct pre_sysdep_segment *sysdep_segments;
  bool have_outdigits;
  int major_revision;
  int minor_revision;
  bool omit_hash_table;
  nls_uint32 hash_tab_size;
  struct mo_file_header header; /* Header of the .mo file to be written.  */
  size_t header_size;
  size_t offset;
  struct string_desc *orig_tab;
  struct string_desc *trans_tab;
  size_t sysdep_tab_offset = 0;
  size_t end_offset;
  char *null;

  /* First pass: Move the static string pairs into an array, for sorting,
     and at the same time, compute the segments of the system dependent
     strings.  */
  msgctid_arr = XNMALLOC (mlp->nitems, char *);
  nstrings = 0;
  msg_arr_allocated = mlp->nitems;
  msg_arr = XNMALLOC (msg_arr_allocated, struct pre_message);
  n_sysdep_strings = 0;
  sysdep_msg_arr = XNMALLOC (mlp->nitems, struct pre_sysdep_message);
  n_sysdep_segments = 0;
  sysdep_segments = NULL;
  have_outdigits = false;
  {
    size_t j;

    for (j = 0; j < mlp->nitems; j++)
      {
        message_ty *mp = mlp->item[j];
        size_t msgctlen;
        char *msgctid;
        struct interval *intervals[2];
        size_t nintervals[2];

        /* Concatenate mp->msgctxt and mp->msgid into msgctid.  */
        msgctlen = (mp->msgctxt != NULL ? strlen (mp->msgctxt) + 1 : 0);
        msgctid = XNMALLOC (msgctlen + strlen (mp->msgid) + 1, char);
        if (mp->msgctxt != NULL)
          {
            memcpy (msgctid, mp->msgctxt, msgctlen - 1);
            msgctid[msgctlen - 1] = MSGCTXT_SEPARATOR;
          }
        strcpy (msgctid + msgctlen, mp->msgid);
        msgctid_arr[j] = msgctid;

        intervals[M_ID] = NULL;
        nintervals[M_ID] = 0;
        intervals[M_STR] = NULL;
        nintervals[M_STR] = 0;

        /* Test if mp contains system dependent strings and thus
           requires the use of the .mo file minor revision 1.  */
        if (possible_format_p (mp->is_format[format_c])
            || possible_format_p (mp->is_format[format_objc]))
          {
            /* Check whether msgid or msgstr contain ISO C 99 <inttypes.h>
               format string directives.  No need to check msgid_plural, because
               it is not accessed by the [n]gettext() function family.  */
            const char *p_end;
            const char *p;

            get_sysdep_c_format_directives (mp->msgid, false,
                                            &intervals[M_ID], &nintervals[M_ID]);
            if (msgctlen > 0)
              {
                struct interval *id_intervals = intervals[M_ID];
                size_t id_nintervals = nintervals[M_ID];

                if (id_nintervals > 0)
                  {
                    unsigned int i;

                    for (i = 0; i < id_nintervals; i++)
                      {
                        id_intervals[i].startpos += msgctlen;
                        id_intervals[i].endpos += msgctlen;
                      }
                  }
              }

            p_end = mp->msgstr + mp->msgstr_len;
            for (p = mp->msgstr; p < p_end; p += strlen (p) + 1)
              {
                struct interval *part_intervals;
                size_t part_nintervals;

                get_sysdep_c_format_directives (p, true,
                                                &part_intervals,
                                                &part_nintervals);
                if (part_nintervals > 0)
                  {
                    size_t d = p - mp->msgstr;
                    unsigned int i;

                    intervals[M_STR] =
                      (struct interval *)
                      xrealloc (intervals[M_STR],
                                (nintervals[M_STR] + part_nintervals)
                                * sizeof (struct interval));
                    for (i = 0; i < part_nintervals; i++)
                      {
                        intervals[M_STR][nintervals[M_STR] + i].startpos =
                          d + part_intervals[i].startpos;
                        intervals[M_STR][nintervals[M_STR] + i].endpos =
                          d + part_intervals[i].endpos;
                      }
                    nintervals[M_STR] += part_nintervals;
                  }
              }
          }

        if (nintervals[M_ID] > 0 || nintervals[M_STR] > 0)
          {
            /* System dependent string pair.  */
            size_t m;

            for (m = 0; m < 2; m++)
              {
                struct pre_sysdep_string *pre =
                  (struct pre_sysdep_string *)
                  xmalloc (xsum (sizeof (struct pre_sysdep_string),
                                 xtimes (nintervals[m],
                                         sizeof (struct pre_segment_pair))));
                const char *str;
                size_t str_len;
                size_t lastpos;
                unsigned int i;

                if (m == M_ID)
                  {
                    str = msgctid; /* concatenation of mp->msgctxt + mp->msgid  */
                    str_len = strlen (msgctid) + 1;
                  }
                else
                  {
                    str = mp->msgstr;
                    str_len = mp->msgstr_len;
                  }

                lastpos = 0;
                pre->segmentcount = nintervals[m];
                for (i = 0; i < nintervals[m]; i++)
                  {
                    size_t length;
                    const char *pointer;
                    size_t r;

                    pre->segments[i].segptr = str + lastpos;
                    pre->segments[i].segsize = intervals[m][i].startpos - lastpos;

                    length = intervals[m][i].endpos - intervals[m][i].startpos;
                    pointer = str + intervals[m][i].startpos;
                    if (length >= 2
                        && pointer[0] == '<' && pointer[length - 1] == '>')
                      {
                        /* Skip the '<' and '>' markers.  */
                        length -= 2;
                        pointer += 1;
                      }

                    for (r = 0; r < n_sysdep_segments; r++)
                      if (sysdep_segments[r].length == length
                          && memcmp (sysdep_segments[r].pointer, pointer, length)
                             == 0)
                        break;
                    if (r == n_sysdep_segments)
                      {
                        n_sysdep_segments++;
                        sysdep_segments =
                          (struct pre_sysdep_segment *)
                          xrealloc (sysdep_segments,
                                    n_sysdep_segments
                                    * sizeof (struct pre_sysdep_segment));
                        sysdep_segments[r].length = length;
                        sysdep_segments[r].pointer = pointer;
                      }

                    pre->segments[i].sysdepref = r;

                    if (length == 1 && *pointer == 'I')
                      have_outdigits = true;

                    lastpos = intervals[m][i].endpos;
                  }
                pre->segments[i].segptr = str + lastpos;
                pre->segments[i].segsize = str_len - lastpos;
                pre->segments[i].sysdepref = SEGMENTS_END;

                sysdep_msg_arr[n_sysdep_strings].str[m] = pre;
              }

            sysdep_msg_arr[n_sysdep_strings].id_plural = mp->msgid_plural;
            sysdep_msg_arr[n_sysdep_strings].id_plural_len =
              (mp->msgid_plural != NULL ? strlen (mp->msgid_plural) + 1 : 0);
            n_sysdep_strings++;
          }
        else
          {
            /* Static string pair.  */
            msg_arr[nstrings].str[M_ID].pointer = msgctid;
            msg_arr[nstrings].str[M_ID].length = strlen (msgctid) + 1;
            msg_arr[nstrings].str[M_STR].pointer = mp->msgstr;
            msg_arr[nstrings].str[M_STR].length = mp->msgstr_len;
            msg_arr[nstrings].id_plural = mp->msgid_plural;
            msg_arr[nstrings].id_plural_len =
              (mp->msgid_plural != NULL ? strlen (mp->msgid_plural) + 1 : 0);
            nstrings++;
          }

        {
          size_t m;

          for (m = 0; m < 2; m++)
            if (intervals[m] != NULL)
              free (intervals[m]);
        }
      }
  }

  /* Second pass: Instantiate the system dependent string pairs and add them to
     the table of static string pairs.  */
  if (!no_redundancy && n_sysdep_strings > 0)
    {
      /* Create a temporary hash table of msg_arr[*].str[M_ID], to guarantee
         fast lookups.  */
      hash_table static_msgids;

      hash_init (&static_msgids, 10);
      {
        size_t i;

        for (i = 0; i < nstrings; i++)
          hash_insert_entry (&static_msgids,
                             msg_arr[i].str[M_ID].pointer,
                             msg_arr[i].str[M_ID].length,
                             NULL);
      }

      size_t ss;

      for (ss = 0; ss < n_sysdep_strings; ss++)
        {
          size_t u;

          for (u = 0; u < SIZEOF (useful_instantiation_rules); u++)
            {
              const struct sysdep_instantiation_rule *instrule =
                &useful_instantiation_rules[u];
              bool supported = true;
              struct pre_string expansion[2];
              size_t m;

              for (m = 0; m < 2; m++)
                {
                  struct pre_sysdep_string *pre = sysdep_msg_arr[ss].str[m];
                  unsigned int segmentcount = pre->segmentcount;
                  size_t expansion_length;
                  char *expansion_pointer;
                  unsigned int i;

                  /* Compute the length of the expansion.  */
                  expansion_length = 0;
                  i = 0;
                  do
                    {
                      expansion_length += pre->segments[i].segsize;

                      size_t r = pre->segments[i].sysdepref;
                      if (r == SEGMENTS_END)
                        break;
                      const char *segment_expansion =
                        get_sysdep_segment_value (sysdep_segments[r], instrule);
                      if (segment_expansion == NULL)
                        {
                          supported = false;
                          break;
                        }
                      expansion_length += strlen (segment_expansion);
                    }
                  while (i++ < segmentcount);
                  if (!supported)
                    break;

                  /* Compute the expansion.  */
                  expansion_pointer = (char *) xmalloc (expansion_length);
                  {
                    char *p = expansion_pointer;

                    i = 0;
                    do
                      {
                        memcpy (p, pre->segments[i].segptr, pre->segments[i].segsize);
                        p += pre->segments[i].segsize;

                        size_t r = pre->segments[i].sysdepref;
                        if (r == SEGMENTS_END)
                          break;
                        const char *segment_expansion =
                          get_sysdep_segment_value (sysdep_segments[r], instrule);
                        if (segment_expansion == NULL)
                          /* Should already have set supported = false above.  */
                          abort ();
                        memcpy (p, segment_expansion, strlen (segment_expansion));
                        p += strlen (segment_expansion);
                      }
                    while (i++ < segmentcount);
                    if (p != expansion_pointer + expansion_length)
                      /* The two loops are not in sync.  */
                      abort ();
                  }

                  expansion[m].length = expansion_length;
                  expansion[m].pointer = expansion_pointer;
                }

              if (supported)
                {
                  /* Don't overwrite existing static string pairs.  */
                  if (hash_insert_entry (&static_msgids,
                                         expansion[M_ID].pointer,
                                         expansion[M_ID].length,
                                         NULL)
                      != NULL)
                    {
                      if (nstrings == msg_arr_allocated)
                        {
                          msg_arr_allocated = 2 * msg_arr_allocated + 1;
                          msg_arr =
                            (struct pre_message *)
                            xreallocarray (msg_arr, msg_arr_allocated,
                                           sizeof (struct pre_message));
                        }
                      msg_arr[nstrings].str[M_ID] = expansion[M_ID];
                      msg_arr[nstrings].str[M_STR] = expansion[M_STR];
                      msg_arr[nstrings].id_plural = sysdep_msg_arr[ss].id_plural;
                      msg_arr[nstrings].id_plural_len = sysdep_msg_arr[ss].id_plural_len;
                      nstrings++;
                    }
                }
            }
        }

      hash_destroy (&static_msgids);
    }

  /* Sort the table according to original string.  */
  if (nstrings > 0)
    qsort (msg_arr, nstrings, sizeof (struct pre_message), compare_id);

  /* We need major revision 1 if there are system dependent strings that use
     "I" because older versions of gettext() crash when this occurs in a .mo
     file.  Otherwise use major revision 0.  */
  major_revision =
    (have_outdigits ? MO_REVISION_NUMBER_WITH_SYSDEP_I : MO_REVISION_NUMBER);

  /* We need minor revision 1 if there are system dependent strings.
     Otherwise we choose minor revision 0 because it's supported by older
     versions of libintl and revision 1 isn't.  */
  minor_revision = (n_sysdep_strings > 0 ? 1 : 0);

  /* In minor revision >= 1, the hash table is obligatory.  */
  omit_hash_table = (no_hash_table && minor_revision == 0);

  /* This should be explained:
     Each string has an associate hashing value V, computed by a fixed
     function.  To locate the string we use open addressing with double
     hashing.  The first index will be V % M, where M is the size of the
     hashing table.  If no entry is found, iterating with a second,
     independent hashing function takes place.  This second value will
     be 1 + V % (M - 2).
     The approximate number of probes will be

       for unsuccessful search:  (1 - N / M) ^ -1
       for successful search:    - (N / M) ^ -1 * ln (1 - N / M)

     where N is the number of keys.

     If we now choose M to be the next prime bigger than 4 / 3 * N,
     we get the values
                         4   and   1.85  resp.
     Because unsuccessful searches are unlikely this is a good value.
     Formulas: [Knuth, The Art of Computer Programming, Volume 3,
                Sorting and Searching, 1973, Addison Wesley]  */
  if (!omit_hash_table)
    {
      /* N is the number of static string pairs (filled in here, below)
         plus the number of system dependent string pairs (filled at runtime,
         in loadmsgcat.c).  */
      hash_tab_size = next_prime (((nstrings + n_sysdep_strings) * 4) / 3);
      /* Ensure M > 2.  */
      if (hash_tab_size <= 2)
        hash_tab_size = 3;
    }
  else
    hash_tab_size = 0;

  /* Third pass: Fill the structure describing the header.  At the same time,
     compute the sizes and offsets of the non-string parts of the file.  */

  /* Magic number.  */
  header.magic = _MAGIC;
  /* Revision number of file format.  */
  header.revision = (major_revision << 16) + minor_revision;

  header_size =
    (minor_revision == 0
     ? offsetof (struct mo_file_header, n_sysdep_segments)
     : sizeof (struct mo_file_header));
  offset = header_size;

  /* Number of static string pairs.  */
  header.nstrings = nstrings;

  /* Offset of table for original string offsets.  */
  header.orig_tab_offset = offset;
  offset += nstrings * sizeof (struct string_desc);
  orig_tab = XNMALLOC (nstrings, struct string_desc);

  /* Offset of table for translated string offsets.  */
  header.trans_tab_offset = offset;
  offset += nstrings * sizeof (struct string_desc);
  trans_tab = XNMALLOC (nstrings, struct string_desc);

  /* Size of hash table.  */
  header.hash_tab_size = hash_tab_size;
  /* Offset of hash table.  */
  header.hash_tab_offset = offset;
  offset += hash_tab_size * sizeof (nls_uint32);

  if (minor_revision >= 1)
    {
      /* Size of table describing system dependent segments.  */
      header.n_sysdep_segments = n_sysdep_segments;
      /* Offset of table describing system dependent segments.  */
      header.sysdep_segments_offset = offset;
      offset += n_sysdep_segments * sizeof (struct sysdep_segment);

      /* Number of system dependent string pairs.  */
      header.n_sysdep_strings = n_sysdep_strings;

      /* Offset of table for original sysdep string offsets.  */
      header.orig_sysdep_tab_offset = offset;
      offset += n_sysdep_strings * sizeof (nls_uint32);

      /* Offset of table for translated sysdep string offsets.  */
      header.trans_sysdep_tab_offset = offset;
      offset += n_sysdep_strings * sizeof (nls_uint32);

      /* System dependent string descriptors.  */
      sysdep_tab_offset = offset;
      {
        size_t m;
        size_t j;

        for (m = 0; m < 2; m++)
          for (j = 0; j < n_sysdep_strings; j++)
            offset += sizeof (struct sysdep_string)
                      + sysdep_msg_arr[j].str[m]->segmentcount
                        * sizeof (struct segment_pair);
      }
    }

  end_offset = offset;


  /* Fourth pass: Write the non-string parts of the file.  At the same time,
     compute the offsets of each string, including the proper alignment.  */

  /* Write the header out.  */
  if (byteswap)
    {
      BSWAP32 (header.magic);
      BSWAP32 (header.revision);
      BSWAP32 (header.nstrings);
      BSWAP32 (header.orig_tab_offset);
      BSWAP32 (header.trans_tab_offset);
      BSWAP32 (header.hash_tab_size);
      BSWAP32 (header.hash_tab_offset);
      if (minor_revision >= 1)
        {
          BSWAP32 (header.n_sysdep_segments);
          BSWAP32 (header.sysdep_segments_offset);
          BSWAP32 (header.n_sysdep_strings);
          BSWAP32 (header.orig_sysdep_tab_offset);
          BSWAP32 (header.trans_sysdep_tab_offset);
        }
    }
  fwrite (&header, header_size, 1, output_file);

  /* Table for original string offsets.  */
  /* Here output_file is at position header.orig_tab_offset.  */

  {
    size_t j;

    for (j = 0; j < nstrings; j++)
      {
        offset = roundup (offset, alignment);
        orig_tab[j].length =
          msg_arr[j].str[M_ID].length + msg_arr[j].id_plural_len;
        orig_tab[j].offset = offset;
        offset += orig_tab[j].length;
        /* Subtract 1 because of the terminating NUL.  */
        orig_tab[j].length--;
      }
    if (byteswap)
      for (j = 0; j < nstrings; j++)
        {
          BSWAP32 (orig_tab[j].length);
          BSWAP32 (orig_tab[j].offset);
        }
    fwrite (orig_tab, nstrings * sizeof (struct string_desc), 1, output_file);
  }

  /* Table for translated string offsets.  */
  /* Here output_file is at position header.trans_tab_offset.  */

  {
    size_t j;

    for (j = 0; j < nstrings; j++)
      {
        offset = roundup (offset, alignment);
        trans_tab[j].length = msg_arr[j].str[M_STR].length;
        trans_tab[j].offset = offset;
        offset += trans_tab[j].length;
        /* Subtract 1 because of the terminating NUL.  */
        trans_tab[j].length--;
      }
    if (byteswap)
      for (j = 0; j < nstrings; j++)
        {
          BSWAP32 (trans_tab[j].length);
          BSWAP32 (trans_tab[j].offset);
        }
    fwrite (trans_tab, nstrings * sizeof (struct string_desc), 1, output_file);
  }

  /* Skip this part when no hash table is needed.  */
  if (!omit_hash_table)
    {
      nls_uint32 *hash_tab;
      size_t j;

      /* Here output_file is at position header.hash_tab_offset.  */

      /* Allocate room for the hashing table to be written out.  */
      hash_tab = XNMALLOC (hash_tab_size, nls_uint32);
      memset (hash_tab, '\0', hash_tab_size * sizeof (nls_uint32));

      /* Insert all values in the hash table, following the algorithm described
         above.  */
      for (j = 0; j < nstrings; j++)
        {
          nls_uint32 hash_val = hash_string (msg_arr[j].str[M_ID].pointer);
          nls_uint32 idx = hash_val % hash_tab_size;

          if (hash_tab[idx] != 0)
            {
              /* We need the second hashing function.  */
              nls_uint32 incr = 1 + (hash_val % (hash_tab_size - 2));

              do
                if (idx >= hash_tab_size - incr)
                  idx -= hash_tab_size - incr;
                else
                  idx += incr;
              while (hash_tab[idx] != 0);
            }

          hash_tab[idx] = j + 1;
        }

      /* Write the hash table out.  */
      if (byteswap)
        for (j = 0; j < hash_tab_size; j++)
          BSWAP32 (hash_tab[j]);
      fwrite (hash_tab, hash_tab_size * sizeof (nls_uint32), 1, output_file);

      free (hash_tab);
    }

  if (minor_revision >= 1)
    {
      /* Here output_file is at position header.sysdep_segments_offset.  */

      {
        struct sysdep_segment *sysdep_segments_tab;
        unsigned int i;

        sysdep_segments_tab =
          XNMALLOC (n_sysdep_segments, struct sysdep_segment);
        for (i = 0; i < n_sysdep_segments; i++)
          {
            offset = roundup (offset, alignment);
            /* The "+ 1" accounts for the trailing NUL byte.  */
            sysdep_segments_tab[i].length = sysdep_segments[i].length + 1;
            sysdep_segments_tab[i].offset = offset;
            offset += sysdep_segments_tab[i].length;
          }

        if (byteswap)
          for (i = 0; i < n_sysdep_segments; i++)
            {
              BSWAP32 (sysdep_segments_tab[i].length);
              BSWAP32 (sysdep_segments_tab[i].offset);
            }
        fwrite (sysdep_segments_tab,
                n_sysdep_segments * sizeof (struct sysdep_segment), 1,
                output_file);

        free (sysdep_segments_tab);
      }

      {
        nls_uint32 *sysdep_tab;
        size_t stoffset;
        size_t m;
        size_t j;

        sysdep_tab = XNMALLOC (n_sysdep_strings, nls_uint32);
        stoffset = sysdep_tab_offset;

        for (m = 0; m < 2; m++)
          {
            /* Here output_file is at position
               m == M_ID  -> header.orig_sysdep_tab_offset,
               m == M_STR -> header.trans_sysdep_tab_offset.  */

            for (j = 0; j < n_sysdep_strings; j++)
              {
                sysdep_tab[j] = stoffset;
                stoffset += sizeof (struct sysdep_string)
                            + sysdep_msg_arr[j].str[m]->segmentcount
                              * sizeof (struct segment_pair);
              }
            /* Write the table for original/translated sysdep string offsets.  */
            if (byteswap)
              for (j = 0; j < n_sysdep_strings; j++)
                BSWAP32 (sysdep_tab[j]);
            fwrite (sysdep_tab, n_sysdep_strings * sizeof (nls_uint32), 1,
                    output_file);
          }

        free (sysdep_tab);
      }

      /* Here output_file is at position sysdep_tab_offset.  */

      {
        size_t m;
        size_t j;

        for (m = 0; m < 2; m++)
          for (j = 0; j < n_sysdep_strings; j++)
            {
              struct pre_sysdep_message *msg = &sysdep_msg_arr[j];
              struct pre_sysdep_string *pre = msg->str[m];
              struct sysdep_string *str =
                (struct sysdep_string *)
                xmalloca (sizeof (struct sysdep_string)
                          + pre->segmentcount * sizeof (struct segment_pair));
              unsigned int i;

              offset = roundup (offset, alignment);
              str->offset = offset;
              for (i = 0; i <= pre->segmentcount; i++)
                {
                  str->segments[i].segsize = pre->segments[i].segsize;
                  str->segments[i].sysdepref = pre->segments[i].sysdepref;
                  offset += str->segments[i].segsize;
                }
              if (m == M_ID && msg->id_plural_len > 0)
                {
                  str->segments[pre->segmentcount].segsize += msg->id_plural_len;
                  offset += msg->id_plural_len;
                }
              if (byteswap)
                {
                  BSWAP32 (str->offset);
                  for (i = 0; i <= pre->segmentcount; i++)
                    {
                      BSWAP32 (str->segments[i].segsize);
                      BSWAP32 (str->segments[i].sysdepref);
                    }
                }
              fwrite (str,
                      sizeof (struct sysdep_string)
                      + pre->segmentcount * sizeof (struct segment_pair),
                      1, output_file);

              freea (str);
            }
      }
    }

  /* Here output_file is at position end_offset.  */

  free (trans_tab);
  free (orig_tab);


  /* Fifth pass: Write the strings.  */

  offset = end_offset;

  /* A few zero bytes for padding.  */
  null = (char *) alloca (alignment);
  memset (null, '\0', alignment);

  /* Now write the original strings.  */
  {
    size_t j;

    for (j = 0; j < nstrings; j++)
      {
        fwrite (null, roundup (offset, alignment) - offset, 1, output_file);
        offset = roundup (offset, alignment);

        fwrite (msg_arr[j].str[M_ID].pointer, msg_arr[j].str[M_ID].length, 1,
                output_file);
        if (msg_arr[j].id_plural_len > 0)
          fwrite (msg_arr[j].id_plural, msg_arr[j].id_plural_len, 1,
                  output_file);
        offset += msg_arr[j].str[M_ID].length + msg_arr[j].id_plural_len;
      }
  }

  /* Now write the translated strings.  */
  {
    size_t j;

    for (j = 0; j < nstrings; j++)
      {
        fwrite (null, roundup (offset, alignment) - offset, 1, output_file);
        offset = roundup (offset, alignment);

        fwrite (msg_arr[j].str[M_STR].pointer, msg_arr[j].str[M_STR].length, 1,
                output_file);
        offset += msg_arr[j].str[M_STR].length;
      }
  }

  if (minor_revision >= 1)
    {
      unsigned int i;
      size_t m;
      size_t j;

      for (i = 0; i < n_sysdep_segments; i++)
        {
          fwrite (null, roundup (offset, alignment) - offset, 1, output_file);
          offset = roundup (offset, alignment);

          fwrite (sysdep_segments[i].pointer, sysdep_segments[i].length, 1,
                  output_file);
          fwrite (null, 1, 1, output_file);
          offset += sysdep_segments[i].length + 1;
        }

      for (m = 0; m < 2; m++)
        for (j = 0; j < n_sysdep_strings; j++)
          {
            struct pre_sysdep_message *msg = &sysdep_msg_arr[j];
            struct pre_sysdep_string *pre = msg->str[m];

            fwrite (null, roundup (offset, alignment) - offset, 1,
                    output_file);
            offset = roundup (offset, alignment);

            for (i = 0; i <= pre->segmentcount; i++)
              {
                fwrite (pre->segments[i].segptr, pre->segments[i].segsize, 1,
                        output_file);
                offset += pre->segments[i].segsize;
              }
            if (m == M_ID && msg->id_plural_len > 0)
              {
                fwrite (msg->id_plural, msg->id_plural_len, 1, output_file);
                offset += msg->id_plural_len;
              }

            free (pre);
          }
    }

  freea (null);
  {
    size_t j;
    for (j = 0; j < mlp->nitems; j++)
      free (msgctid_arr[j]);
  }
  free (sysdep_msg_arr);
  free (msg_arr);
  free (msgctid_arr);
}


int
msgdomain_write_mo (message_list_ty *mlp,
                    const char *domain_name,
                    const char *file_name,
                    const char *input_file)
{
  /* If no entry for this domain don't even create the file.  */
  if (mlp->nitems != 0)
    {
      if (!no_convert_to_utf8)
        {
          /* Convert the messages to UTF-8.
             This is necessary because the *gettext functions in musl libc
             assume that both the locale encoding and the .mo encoding is UTF-8.
             It is also helpful for performance on glibc systems, since most
             locales nowadays have UTF-8 as locale encoding, whereas some PO
             files still are encoded in EUC-JP or so.  */
          iconv_message_list (mlp, NULL, po_charset_utf8, input_file);
        }

      /* Support for "reproducible builds": Delete information that may vary
         between builds in the same conditions.  */
      message_list_delete_header_field (mlp, "POT-Creation-Date:");

      if (strcmp (domain_name, "-") == 0)
        {
          FILE *output_file = stdout;
          SET_BINARY (fileno (output_file));

          write_table (output_file, mlp);

          /* Make sure nothing went wrong.  */
          if (fwriteerror (output_file))
            error (EXIT_FAILURE, errno, _("error while writing \"%s\" file"),
                   file_name);
        }
      else
        {
          /* Supersede, don't overwrite, the output file.  Otherwise, processes
             that are currently using (via mmap!) the output file could crash
             (through SIGSEGV or SIGBUS).  */
          struct supersede_final_action action;
          FILE *output_file =
            fopen_supersede (file_name, "wb", true, true, &action);
          if (output_file == NULL)
            {
              error (0, errno, _("error while opening \"%s\" for writing"),
                     file_name);
              return 1;
            }

          write_table (output_file, mlp);

          /* Make sure nothing went wrong.  */
          if (fwriteerror_supersede (output_file, &action))
            error (EXIT_FAILURE, errno, _("error while writing \"%s\" file"),
                   file_name);
        }
    }

  return 0;
}
