/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <sys/param.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>    // readdir()
#include <sys/param.h> // MAXPATHLEN
#include <pthread.h>   // mutex
#include <libgen.h>    // dirname
#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <errno.h>     // errno
#include <fcntl.h>     // open

#include "util.h"
#include "dbe_structs.h"
#include "StringBuilder.h"
#include "StringMap.h"      // For directory names
#include "Application.h"    // Only for get_prog_name
#include "vec.h"

void
tsadd (timestruc_t *result, timestruc_t *time)
{
  // This routine will add "time" to "result".
  result->tv_sec += time->tv_sec;
  result->tv_nsec += time->tv_nsec;
  if (result->tv_nsec >= NANOSEC)
    {
      result->tv_nsec -= NANOSEC;
      result->tv_sec++;
    }
}

void
tssub (timestruc_t *result, timestruc_t *time1, timestruc_t *time2)
{
  // This routine will store "time1" - "time2" in "result".

  if (time1->tv_nsec >= time2->tv_nsec)
    {
      result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
      if (time1->tv_sec >= time2->tv_sec)
	result->tv_sec = time1->tv_sec - time2->tv_sec;
      else
	{
	  result->tv_sec = -1;
	  result->tv_nsec = 0;
	}
    }
  else
    {
      result->tv_nsec = time1->tv_nsec + NANOSEC - time2->tv_nsec;
      if (time1->tv_sec - 1 >= time2->tv_sec)
	result->tv_sec = time1->tv_sec - 1 - time2->tv_sec;
      else
	{
	  result->tv_sec = -1;
	  result->tv_nsec = 0;
	}
    }
}

int
tscmp (timestruc_t *time1, timestruc_t *time2)
{
  // This routine will return 1 if "time1" is greater than "time2"
  // and 0 if "time1" is equal to "time2" and -1 otherwise.
  if (time1->tv_sec == time2->tv_sec)
    return time1->tv_nsec > time2->tv_nsec ? 1 :
	  time1->tv_nsec == time2->tv_nsec ? 0 : -1;
  else
    return time1->tv_sec > time2->tv_sec ? 1 : -1;
}

void
int_max (int *maximum, int count)
{
  if (count > *maximum)
    *maximum = count;
}

double
TValue::to_double ()
{
  switch (tag)
    {
    case VT_DOUBLE:
      return (double) d;
    case VT_INT:
      return (double) i;
    case VT_ULLONG:
      return (double) ull;
    case VT_LLONG:
    case VT_ADDRESS:
      return (double) ll;
    case VT_FLOAT:
      return (double) f;
    case VT_SHORT:
      return (double) s;
    default:
      return 0.0;
    }
}

int
TValue::to_int ()
{
  switch (tag)
    {
    case VT_DOUBLE:
      return (int) d;
    case VT_INT:
      return (int) i;
    case VT_ULLONG:
      return (int) ull;
    case VT_LLONG:
    case VT_ADDRESS:
      return (int) ll;
    case VT_FLOAT:
      return (int) f;
    case VT_SHORT:
      return (int) s;
    default:
      return 0;
    }
}

size_t
TValue::get_len ()
{
  char buf[256];
  return strlen (to_str (buf, sizeof (buf)));
}

char *
TValue::to_str (char *str, size_t strsz)
{
  switch (tag)
    {
    case VT_DOUBLE:
      if (d == 0.)
	{
	  if (sign)
	    snprintf (str, strsz, NTXT ("+0.   "));
	  else
	    snprintf (str, strsz, NTXT ("0.   "));
	}
      else if (sign)
	snprintf (str, strsz, NTXT ("%+.3lf"), d);
      else
	snprintf (str, strsz, NTXT ("%.3lf"), d);
      break;
    case VT_INT:
      snprintf (str, strsz, NTXT ("%u"), i);
      break;
    case VT_LLONG:
      if (sign)
	snprintf (str, strsz, NTXT ("%+lld"), ll);
      else
	snprintf (str, strsz, NTXT ("%lld"), ll);
      break;
    case VT_ULLONG:
      snprintf (str, strsz, NTXT ("%llu"), ll);
      break;
    case VT_ADDRESS:
      snprintf (str, strsz, NTXT ("%u:0x%08x"), ADDRESS_SEG (ll), ADDRESS_OFF (ll));
      break;
    case VT_FLOAT:
      snprintf (str, strsz, NTXT ("%.3f"), f);
      break;
    case VT_SHORT:
      snprintf (str, strsz, NTXT ("%hu"), s);
      break;
    case VT_LABEL:
      return l; // 'str' is not used !!!
    default:
      *str = '\0';
      break;
    }

  return str;
}

void
TValue::make_delta (TValue *v1, TValue *v2)
{
  assert (v1->tag == v2->tag);
  tag = v1->tag;
  sign = true;
  switch (v1->tag)
    {
    case VT_INT:
      i = v1->i - v2->i;
      break;
    case VT_LLONG:
      ll = v1->ll - v2->ll;
      break;
    case VT_ULLONG:
    case VT_ADDRESS:
      tag = VT_LLONG;
      ll = (long long) (v1->ull - v2->ull);
      break;
    case VT_FLOAT:
      f = v1->f - v2->f;
      break;
    case VT_DOUBLE:
      d = v1->d - v2->d;
      break;
    default:
      assert (0);
      break;
    }
}

void
TValue::make_ratio (TValue *v1, TValue *v2)
{
  assert (v1->tag == v2->tag);
  double x1 = v1->to_double ();
  double x2 = v2->to_double ();
  sign = false;
  if (x1 == 0.)
    {
      // if the numerator is 0, the ratio is 1. or 0. only
      d = (x2 == 0.) ? 1. : 0.;
      tag = VT_DOUBLE;
    }
  else
    {
      // EUGENE replace 99.999 with a variable that is known by both DBE and GUI
      if (x1 > 99.999 * x2)
	{
	  l = dbe_strdup (">99.999");
	  tag = VT_LABEL;
	}
      else if (x1 < -99.999 * x2)
	{
	  l = dbe_strdup ("<-99.999");
	  tag = VT_LABEL;
	}
      else
	{
	  d = x1 / x2;
	  tag = VT_DOUBLE;
	}
    }
}

int
TValue::compare (TValue *v)
{
  if (tag != v->tag)
    { // Only for comparison (Ratio)
      if (tag == VT_LABEL)
	{
	  if (v->tag == VT_LABEL)
	    return strcoll (l, v->l);
	  return 1;
	}
      if (v->tag == VT_LABEL)
	return -1;
      return ll < v->ll ? -1 : (ll == v->ll ? 0 : 1);
    }
  switch (tag)
    {
    case VT_SHORT:
      return s < v->s ? -1 : (s == v->s ? 0 : 1);
    case VT_INT:
      return i < v->i ? -1 : (i == v->i ? 0 : 1);
    case VT_FLOAT:
      return f < v->f ? -1 : (f == v->f ? 0 : 1);
    case VT_DOUBLE:
      return d < v->d ? -1 : (d == v->d ? 0 : 1);
    case VT_LABEL:
      return strcoll (l, v->l);
    case VT_LLONG:
    case VT_ULLONG:
    case VT_ADDRESS:
    case VT_HRTIME:
    default:
      return (ll < v->ll) ? -1 : ((ll == v->ll) ? 0 : 1);
    }
}

char *
strstr_r (char *s1, const char *s2)
{
  char *str = NULL;
  for (char *s = s1; s;)
    {
      s = strstr (s, s2);
      if (s)
	{
	  str = s;
	  s++;
	}
    }
  return str;
}

// reversal order of strpbrk

char *
strrpbrk (const char *string, const char *brkset)
{
  const char *p;
  const char *s;
  for (s = string + strlen (string) - 1; s >= string; s--)
    {
      for (p = brkset; *p != '\0' && *p != *s; ++p)
	;
      if (*p != '\0')
	return ((char *) s);
    }
  return NULL;
}

char *
read_line (FILE *fptr)
{
  // get an input line, no size limit
  int line_sz = 128; // starting size
  char *line = (char *) malloc (line_sz);

  // read as much of the line as will fit in memory
  line[0] = 0;
  int len = 0;
  for (;;)
    {
      while (fgets (line + len, line_sz - len, fptr) != NULL)
	{
	  len = (int) strlen (line);
	  if (len == 0 || line[len - 1] == '\n')
	    break;
	  // increase the buffer
	  char *lineNew = (char *) malloc (2 * line_sz);
	  strncpy (lineNew, line, line_sz);
	  lineNew[line_sz] = '\0';
	  free (line);
	  line = lineNew;
	  line_sz *= 2;
	  if (line == NULL)
	    {
	      fprintf (stderr, GTXT ("   Line too long -- out of memory; exiting\n"));
	      exit (1);
	    }
	}
      if (len == 0)
	{
	  free (line);
	  return NULL;
	}
      // see if there's a continuation line
      if ((len >= 2) && (line[len - 1] == '\n') && (line[len - 2] == '\\'))
	{
	  // remove the trailing \ and the \n, and keep going
	  line[len - 2] = 0;
	  len -= 2;
	}
      else
	break;
    }
  return line; // expecting the caller to free it
}

Vector<char *> *
split_str (char *str, char delimiter)
{
  Vector<char *> *v = new Vector<char *>;
  for (char *s = str; s;)
    {
      if (*s == '"')
	{
	  char *next_s = NULL;
	  char *tok = parse_qstring (s, &next_s);
	  if (tok && *tok != '\0')
	    v->append (tok);
	  if (*next_s)
	    s = next_s + 1;
	  else
	    s = NULL;
	}
      else
	{
	  char *next_s = strchr (s, delimiter);
	  if (next_s)
	    {
	      if (next_s != s)
		v->append (dbe_strndup (s, next_s - s));
	      s = next_s + 1;
	    }
	  else
	    {
	      if (*s != '\0')
		v->append (dbe_strdup (s));
	      s = NULL;
	    }
	}
    }
  return v;
}

// get quoted string
char *
parse_qstring (char *in_str, char **endptr)
{
  int i;
  char c, c2;
  char term;
  char csnum[2 * MAXPATHLEN];

  // Skip any leading blanks or tabs
  while (*in_str == '\t' || *in_str == ' ')
    in_str++;

  int gtxt = 0;
  if (*in_str == 'G' && *(in_str + 1) == 'T' && *(in_str + 2) == 'X'
      && *(in_str + 3) == 'T' && *(in_str + 4) == '(')
    {
      gtxt = 1;
      in_str += 5;
    }
  // non-quoted string
  if (*in_str == '"')
    term = '"';
  else if (*in_str == '\'')
    term = '\'';
  else
    return strtok_r (in_str, NTXT (" "), endptr);

  StringBuilder sb;
  while ((c = *(++in_str)) != '\0')
    {
      if (c == term) // the closing quote
	break;
      if (c == '\\')
	{ // handle any escaped characters
	  c2 = *(++in_str);
	  switch (c2)
	    {
	    case '\"':
	      sb.append ('\"');
	      break;
	    case '\'':
	      sb.append ('\'');
	      break;
	    case '\\':
	      sb.append ('\\');
	      break;
	    case 't':
	      sb.append ('\t');
	      break;
	    case 'r':
	      sb.append ('\r');
	      break;
	    case 'b':
	      sb.append ('\b');
	      break;
	    case 'f':
	      sb.append ('\f');
	      break;
	    case 'n':
	      sb.append ('\n');
	      break;
	    default:
	      if ((c2 >= '0') && (c2 <= '9'))
		{
		  for (i = 0; i < MAXPATHLEN; i++)
		    {
		      if (((c2 < '0') || (c2 > '9')) && (c2 != 'x') &&
			  ((c2 < 'a') || (c2 > 'f')) &&
			  ((c2 < 'A') || (c2 > 'F')))
			{
			  csnum[i] = '\0';
			  --in_str;
			  break;
			}
		      else
			{
			  csnum[i] = c2;
			  c2 = *(++in_str);
			}
		    }
		  sb.append ((char) strtoul (csnum, endptr, 0));
		}
	      else
		sb.append (c2);
	      break;
	    }
	}
      else
	sb.append (c);
    }
  if (c == term && gtxt && *in_str == ')')
    in_str++;
  if (*in_str == '\0')
    *endptr = in_str;
  else
    *endptr = in_str + 1;
  return sb.toString ();
}

// parse a file name of the form name`name2`
// returns name
// stores the pointer to named in fcontext
// returns NULL if the string is not properly formatted
char *
parse_fname (char *in_str, char **fcontext)
{
  *fcontext = NULL;
  int ch = '`';
  if (in_str == NULL)
    return NULL;
  char *copy = strdup (in_str);
  char *p = strchr (copy, ch);
  if (p != NULL)
    {
      // yes, there's an embedded file name
      *p = '\0';
      p++;
      // now find the terminating single quote
      char *p1 = strchr (p, ch);
      if (p1 == NULL)
	{
	  // if we don't have the closing `, the format is incorrect
	  free (copy);
	  return NULL;
	}
      //remove the closing quote
      *p1 = '\0';
      // see if there's anything following it
      if (*(p1 + 1) != 0)
	{
	  // error in format
	  free (copy);
	  return NULL;
	}
      free (*fcontext);
      *fcontext = strdup (p);
    }
  return copy;
}

int
get_paren (const char *name)
{
  char buf[8192];
  char *ptr;
  int temp_level1, temp_level2;

  temp_level1 = temp_level2 = 0;
  snprintf (buf, sizeof (buf), NTXT ("%s"), name);
  while ((ptr = strrpbrk (buf, "><)(")) != NULL)
    {
      if (*ptr == '>')
	temp_level1++;
      else if (*ptr == '<')
	temp_level1--;
      else if (*ptr == ')')
	temp_level2++;
      else
	{
	  temp_level2--;
	  if (temp_level1 <= 0 && temp_level2 <= 0)
	    return (int) (ptr - buf);
	}
      *ptr = '\0';
    }
  return -1;
}

// CRC-64 based on x^64 + x^11 + x^2 + x + 1 polynomial.
// This algorithm doesn't perform well but is short and
// readable. We currently use it for a small amount of
// short strings. Should this change, another algorithm
// with better performance is to be used instead.
static uint64_t masks[256] = {
  /*   0 */ 0x000000, 0x000807, 0x00100e, 0x001809, 0x00201c, 0x00281b,
  /*   6 */ 0x003012, 0x003815, 0x004038, 0x00483f, 0x005036, 0x005831,
  /*  12 */ 0x006024, 0x006823, 0x00702a, 0x00782d, 0x008070, 0x008877,
  /*  18 */ 0x00907e, 0x009879, 0x00a06c, 0x00a86b, 0x00b062, 0x00b865,
  /*  24 */ 0x00c048, 0x00c84f, 0x00d046, 0x00d841, 0x00e054, 0x00e853,
  /*  30 */ 0x00f05a, 0x00f85d, 0x0100e0, 0x0108e7, 0x0110ee, 0x0118e9,
  /*  36 */ 0x0120fc, 0x0128fb, 0x0130f2, 0x0138f5, 0x0140d8, 0x0148df,
  /*  42 */ 0x0150d6, 0x0158d1, 0x0160c4, 0x0168c3, 0x0170ca, 0x0178cd,
  /*  48 */ 0x018090, 0x018897, 0x01909e, 0x019899, 0x01a08c, 0x01a88b,
  /*  54 */ 0x01b082, 0x01b885, 0x01c0a8, 0x01c8af, 0x01d0a6, 0x01d8a1,
  /*  60 */ 0x01e0b4, 0x01e8b3, 0x01f0ba, 0x01f8bd, 0x0201c0, 0x0209c7,
  /*  66 */ 0x0211ce, 0x0219c9, 0x0221dc, 0x0229db, 0x0231d2, 0x0239d5,
  /*  72 */ 0x0241f8, 0x0249ff, 0x0251f6, 0x0259f1, 0x0261e4, 0x0269e3,
  /*  78 */ 0x0271ea, 0x0279ed, 0x0281b0, 0x0289b7, 0x0291be, 0x0299b9,
  /*  84 */ 0x02a1ac, 0x02a9ab, 0x02b1a2, 0x02b9a5, 0x02c188, 0x02c98f,
  /*  90 */ 0x02d186, 0x02d981, 0x02e194, 0x02e993, 0x02f19a, 0x02f99d,
  /*  96 */ 0x030120, 0x030927, 0x03112e, 0x031929, 0x03213c, 0x03293b,
  /* 102 */ 0x033132, 0x033935, 0x034118, 0x03491f, 0x035116, 0x035911,
  /* 108 */ 0x036104, 0x036903, 0x03710a, 0x03790d, 0x038150, 0x038957,
  /* 114 */ 0x03915e, 0x039959, 0x03a14c, 0x03a94b, 0x03b142, 0x03b945,
  /* 120 */ 0x03c168, 0x03c96f, 0x03d166, 0x03d961, 0x03e174, 0x03e973,
  /* 126 */ 0x03f17a, 0x03f97d, 0x040380, 0x040b87, 0x04138e, 0x041b89,
  /* 132 */ 0x04239c, 0x042b9b, 0x043392, 0x043b95, 0x0443b8, 0x044bbf,
  /* 138 */ 0x0453b6, 0x045bb1, 0x0463a4, 0x046ba3, 0x0473aa, 0x047bad,
  /* 144 */ 0x0483f0, 0x048bf7, 0x0493fe, 0x049bf9, 0x04a3ec, 0x04abeb,
  /* 150 */ 0x04b3e2, 0x04bbe5, 0x04c3c8, 0x04cbcf, 0x04d3c6, 0x04dbc1,
  /* 156 */ 0x04e3d4, 0x04ebd3, 0x04f3da, 0x04fbdd, 0x050360, 0x050b67,
  /* 162 */ 0x05136e, 0x051b69, 0x05237c, 0x052b7b, 0x053372, 0x053b75,
  /* 168 */ 0x054358, 0x054b5f, 0x055356, 0x055b51, 0x056344, 0x056b43,
  /* 174 */ 0x05734a, 0x057b4d, 0x058310, 0x058b17, 0x05931e, 0x059b19,
  /* 180 */ 0x05a30c, 0x05ab0b, 0x05b302, 0x05bb05, 0x05c328, 0x05cb2f,
  /* 186 */ 0x05d326, 0x05db21, 0x05e334, 0x05eb33, 0x05f33a, 0x05fb3d,
  /* 192 */ 0x060240, 0x060a47, 0x06124e, 0x061a49, 0x06225c, 0x062a5b,
  /* 198 */ 0x063252, 0x063a55, 0x064278, 0x064a7f, 0x065276, 0x065a71,
  /* 204 */ 0x066264, 0x066a63, 0x06726a, 0x067a6d, 0x068230, 0x068a37,
  /* 210 */ 0x06923e, 0x069a39, 0x06a22c, 0x06aa2b, 0x06b222, 0x06ba25,
  /* 216 */ 0x06c208, 0x06ca0f, 0x06d206, 0x06da01, 0x06e214, 0x06ea13,
  /* 222 */ 0x06f21a, 0x06fa1d, 0x0702a0, 0x070aa7, 0x0712ae, 0x071aa9,
  /* 228 */ 0x0722bc, 0x072abb, 0x0732b2, 0x073ab5, 0x074298, 0x074a9f,
  /* 234 */ 0x075296, 0x075a91, 0x076284, 0x076a83, 0x07728a, 0x077a8d,
  /* 240 */ 0x0782d0, 0x078ad7, 0x0792de, 0x079ad9, 0x07a2cc, 0x07aacb,
  /* 246 */ 0x07b2c2, 0x07bac5, 0x07c2e8, 0x07caef, 0x07d2e6, 0x07dae1,
  /* 252 */ 0x07e2f4, 0x07eaf3, 0x07f2fa, 0x07fafd
};

uint64_t
crc64 (const char *str, size_t len)
{
  uint64_t res = 0LL;
  for (size_t i = 0; i < len; i++)
    {
      unsigned char b = (unsigned char) ((res >> 56) ^ *str++);
      res = res << 8;
      res ^= masks [b];
    }
  return res;
}

/**
 * Canonize path inside the string provided by the argument
 * @param path
 * @return path
 */
char *
canonical_path (char *path)
{
  char *s1, *s2;
  if (!path)
    return path;
  s1 = path;
  s2 = path;
  while (*s1)
    {
      if (*s1 == '.' && s1[1] == '/')
	{ // remove .///
	  for (s1++; *s1; s1++)
	    if (*s1 != '/')
	      break;
	}
      else if (*s1 == '/')
	{ // replace /// with /
	  *(s2++) = *s1;
	  for (s1++; *s1; s1++)
	    if (*s1 != '/')
	      break;
	}
      else
	{
	  while (*s1)
	    { // copy file or directory name
	      if (*s1 == '/')
		break;
	      *(s2++) = *(s1++);
	    }
	}
    }
  *s2 = 0;
  if (s2 != path && (s2 - 1) != path && s2[-1] == '/')  // remove last /
    *(s2 - 1) = 0;
  return path;
}

char *
get_relative_path (char *name)
{
  if (*name == '/' && theApplication)
    {
      char *cwd = theApplication->get_cur_dir ();
      if (cwd)
	{
	  size_t len = strlen (cwd);
	  if (len > 0 && len < strlen (name) && name[len] == '/'
	      && strncmp (cwd, name, len) == 0)
	    {
	      for (name += len + 1; *name == '/'; name++)
		;
	      return name;
	    }
	}
    }
  return name;
}

/**
 * Generate a relative link name from path_from to path_to
 * Example:
 * path_from=a/b/c/d
 * path_to=a/b/e/f/g
 * lname=../../e/f/g
 * @param path_to
 * @param path_from
 * @return lname - relative link
 */
char *
get_relative_link (const char *path_from, const char *path_to)
{
  if (!path_to)
    path_to = ".";
  if (!path_from)
    path_from = ".";
  char *s1 = dbe_strdup (path_to);
  s1 = canonical_path (s1);
  char *s2 = dbe_strdup (path_from);
  s2 = canonical_path (s2);
  long l = dbe_sstrlen (s1);
  // try to find common directories
  int common_slashes = 0;
  int last_common_slash = -1;
  for (int i = 0; i < l; i++)
    {
      if (s1[i] != s2[i]) break;
      if (s1[i] == 0) break;
      if (s1[i] == '/')
	{
	  common_slashes++;
	  last_common_slash = i;
	}
    }
  // find slashes in remaining path_to
  int slashes = 0;
  for (int i = last_common_slash + 1; i < l; i++)
    {
      if (s1[i] == '/')
	{
	  // Exclude "/./" case
	  if (i > last_common_slash + 2)
	    {
	      if (s1[i - 1] == '.' && s1[i - 2] == '/')
		continue;
	    }
	  else if (i > 0 && s1[i - 1] == '.')
	    continue;
	  slashes++;
	}
    }
  // generate relative path
  StringBuilder sb;
  for (int i = 0; i < slashes; i++)
    sb.append ("../");
  sb.append (s2 + last_common_slash + 1);
  char *lname = sb.toString ();
  free (s1);
  free (s2);
  return lname;
}

char *
get_prog_name (int basename)
{
  char *nm = NULL;
  if (theApplication)
    {
      nm = theApplication->get_name ();
      if (nm && basename)
	nm = get_basename (nm);
    }
  return nm;
}

char *
dbe_strndup (const char *str, size_t len)
{
  if (str == NULL)
    return NULL;
  char *s = (char *) malloc (len + 1);
  strncpy (s, str, len);
  s[len] = '\0';
  return s;
}

char *
dbe_sprintf (const char *fmt, ...)
{
  char buffer[256];
  int buf_size;
  va_list vp;

  va_start (vp, fmt);
  buf_size = vsnprintf (buffer, sizeof (buffer), fmt, vp) + 1;
  va_end (vp);
  if (buf_size < (int) sizeof (buffer))
    {
      if (buf_size <= 1)
	buffer[0] = 0;
      return strdup (buffer);
    }

  va_start (vp, fmt);
  char *buf = (char *) malloc (buf_size);
  vsnprintf (buf, buf_size, fmt, vp);
  va_end (vp);
  return buf;
}

ssize_t
dbe_write (int f, const char *fmt, ...)
{
  char buffer[256];
  int buf_size;
  va_list vp;

  va_start (vp, fmt);
  buf_size = vsnprintf (buffer, sizeof (buffer), fmt, vp) + 1;
  va_end (vp);
  if (buf_size < (int) sizeof (buffer))
    {
      if (buf_size <= 1)
	buffer[0] = 0;
      return write (f, buffer, strlen (buffer));
    }

  va_start (vp, fmt);
  char *buf = (char *) malloc (buf_size);
  vsnprintf (buf, buf_size, fmt, vp);
  va_end (vp);
  ssize_t val = write (f, buf, strlen (buf));
  free (buf);
  return val;
}

/* Worker Threads to avoid hanging on file servers */

/*
 * Thread states
 */
enum
{
  THREAD_START,
  THREAD_STARTED,
  THREAD_CANCEL,
  THREAD_CANCELED,
  THREAD_CREATE,
  THREAD_NOT_CREATED,
  THREAD_FINISHED
};

/*
 * Communication structure
 */
struct worker_thread_info
{
  pthread_t thread_id;      /* ID returned by pthread_create() */
  int thread_num;           /* Application-defined thread # */
  volatile int control;     /* Thread state */
  volatile int result;      /* Return status */
  struct stat64 statbuf;    /* File info from stat64() */
  const char *path;         /* File */
};

static pthread_mutex_t worker_thread_lock = PTHREAD_MUTEX_INITIALIZER;
static int worker_thread_number = 0;
/**
 * Call stat64() on current worker thread
 * Check if control is not THREAD_CANCEL
 * If control is THREAD_CANCEL return (exit thread)
 * @param *wt_info
 */
static void *
dbe_stat_on_thread (void *arg)
{
  struct worker_thread_info *wt_info = (struct worker_thread_info *) arg;
  pthread_mutex_lock (&worker_thread_lock);
  {
    if (wt_info->control != THREAD_START)
      {
	// Already too late
	pthread_mutex_unlock (&worker_thread_lock);
	return 0;
      }
    wt_info->control = THREAD_STARTED;
  }
  pthread_mutex_unlock (&worker_thread_lock);
  const char * path = wt_info->path;
  int st = stat64 (path, &(wt_info->statbuf));
  pthread_mutex_lock (&worker_thread_lock);
  {
    if (wt_info->control == THREAD_CANCEL)
      {
	// Too late.
	pthread_mutex_unlock (&worker_thread_lock);
	free (wt_info);
	return 0;
      }
    wt_info->result = st;
    wt_info->control = THREAD_FINISHED;
  }
  pthread_mutex_unlock (&worker_thread_lock);
  return 0;
}

/**
 * Create a worker thread to call specified function
 * Wait for its result, but not longer than 5 seconds
 * If the timeout happens, tell the thread to cancel
 * @param path
 * @param wt_info
 * @return thread state
 */
static int
dbe_dispatch_on_thread (const char *path, struct worker_thread_info *wt_info)
{
  wt_info->result = 0;
  wt_info->control = THREAD_START;
  pthread_attr_t attr;
  /* Initialize thread creation attributes */
  int res = pthread_attr_init (&attr);
  if (res != 0)
    {
      wt_info->control = THREAD_NOT_CREATED;
      return THREAD_NOT_CREATED;
    }
  wt_info->thread_id = 0;
  wt_info->path = path;
  // Lock
  pthread_mutex_lock (&worker_thread_lock);
  worker_thread_number++;
  wt_info->thread_num = worker_thread_number;
  // Unlock
  pthread_mutex_unlock (&worker_thread_lock);
  // Create thread
  res = pthread_create (&wt_info->thread_id, &attr, &dbe_stat_on_thread, wt_info);
  if (res != 0)
    {
      wt_info->control = THREAD_NOT_CREATED;
      pthread_attr_destroy (&attr);
      return THREAD_NOT_CREATED;
    }
  // Wait for the thread to finish
  res = 0;
  useconds_t maxusec = 5000000; // 5 seconds
  useconds_t deltausec = 1000; // 1 millisecond
  int max = maxusec / deltausec;
  for (int i = 0; i < max; i++)
    {
      if (THREAD_FINISHED == wt_info->control)
	break; // We are done
      usleep (deltausec);
    }
  // Lock
  pthread_mutex_lock (&worker_thread_lock);
  if (THREAD_FINISHED != wt_info->control)
    {
      // Cancel thread
      wt_info->control = THREAD_CANCEL; // Cannot use wt_info after that!
      res = THREAD_CANCEL;
    }
  // Unlock
  pthread_mutex_unlock (&worker_thread_lock);
  // Destroy the thread attributes object, since it is no longer needed
  pthread_attr_destroy (&attr);
  // Report that thread was canceled
  if (THREAD_CANCEL == res)
    return res; /* Cannot free memory allocated by thread */
  // Free all thread resources
  void *resources = 0;
  res = pthread_join (wt_info->thread_id, &resources);
  free (resources); /* Free memory allocated by thread */
  return THREAD_FINISHED;
}

static pthread_mutex_t dirnames_lock = PTHREAD_MUTEX_INITIALIZER;
static Map<const char*, int> *dirnamesMap = NULL;

#define DIR_STATUS_EXISTS 0
#define DIR_STATUS_UNKNOWN 2

/**
 * Check if this directory name is known
 * Return:
 * @param path
 *  0 - known, exists
 *  1 - known, does not exist
 *  2 - not known
 */
static int
check_dirname (const char *path)
{
  pthread_mutex_lock (&dirnames_lock);
  if (NULL == dirnamesMap)
    dirnamesMap = new StringMap<int>(128, 128);
  pthread_mutex_unlock (&dirnames_lock);
  int res = DIR_STATUS_UNKNOWN;
  if (path && *path)
    {
      char *fn = dbe_strdup (path);
      char *dn = dirname (fn);
      if (dn && *dn)
	res = dirnamesMap->get (dn);
      free (fn);
    }
  return res;
}

/**
 * Save directory name and its status
 * @param path
 * @param status
 * @return
 */
static void
extract_and_save_dirname (const char *path, int status)
{
  pthread_mutex_lock (&dirnames_lock);
  if (NULL == dirnamesMap)
    dirnamesMap = new StringMap<int>(128, 128);
  pthread_mutex_unlock (&dirnames_lock);
  char *fn = dbe_strdup (path);
  if (fn && *fn != 0)
    {
      char *dn = dirname (fn);
      if (dn && (*dn != 0))
	{
	  int st = 0; // exists
	  if (0 != status)
	    st = 1; // does not exist
	  dirnamesMap->put (dn, st);
	}
    }
  free (fn);
}

// get status for specified file
static int
dbe_stat_internal (const char *path, struct stat64 *sbuf, bool file_only)
{
  struct stat64 statbuf;
  int dir_status = check_dirname (path);
  if (dir_status == DIR_STATUS_UNKNOWN)
    {
      // Try to use a worker thread
      if (theApplication->get_number_of_worker_threads () > 0)
	{
	  struct worker_thread_info *wt_info;
	  wt_info = (worker_thread_info *) calloc (1, sizeof (worker_thread_info));
	  if (wt_info != NULL)
	    {
	      int res = dbe_dispatch_on_thread (path, wt_info);
	      if (THREAD_FINISHED == res)
		{
		  int st = wt_info->result;
		  extract_and_save_dirname (path, st);
		  if (st == 0 && file_only)
		    if (S_ISREG ((wt_info->statbuf).st_mode) == 0)
		      st = -1; // It is not a regular file
		  if (sbuf != NULL)
		    *sbuf = wt_info->statbuf;
		  free (wt_info);
		  return st;
		}
	      else
		{
		  if (THREAD_CANCEL == res)
		    {
		      // Worker thread hung. Cannot free wt_info.
		      // Allocated memory will be freed by worker thread.
		      // save directory
		      extract_and_save_dirname (path, 1);
		      return 1; // stat64 failed
		    }
		  else  // THREAD_NOT_CREATED - continue on current thread
		    free (wt_info);
		}
	    }
	}
    }
  else if (dir_status != DIR_STATUS_EXISTS)
    return -1; // does not exist
  if (sbuf == NULL)
    sbuf = &statbuf;
  int st = stat64 (path, sbuf);
  Dprintf (DEBUG_DBE_FILE, NTXT ("dbe_stat %d '%s'\n"), st, path);
  if (st == -1)
    return -1;
  else if (file_only && S_ISREG (sbuf->st_mode) == 0)
    return -1; // It is not ordinary file
  return st;
}

// get status for the regular file

int
dbe_stat_file (const char *path, struct stat64 *sbuf)
{
  int res = dbe_stat_internal (path, sbuf, true);
  return res;
}

// get status for specified file

int
dbe_stat (const char *path, struct stat64 *sbuf)
{
  int res = dbe_stat_internal (path, sbuf, false);
  return res;
}

/**
 * Reads directory and prepares list of files according to the specified format
 * Supported formats:
 * "/bin/ls -a" - see 'man ls' for details
 * "/bin/ls -aF" - see 'man ls' for details
 * @param path
 * @param format
 * @return char * files
 */
char *
dbe_read_dir (const char *path, const char *format)
{
  StringBuilder sb;
  DIR *dir = opendir (path);
  if (dir == NULL)
    return sb.toString ();
  int format_aF = 0;
  if (!strcmp (format, NTXT ("/bin/ls -aF")))
    format_aF = 1;
  struct dirent *entry = NULL;
  if (format != NULL)
    {
      while ((entry = readdir (dir)) != NULL)
	{
	  sb.append (entry->d_name);
	  if (format_aF)
	    {
	      const char *attr = NTXT ("@"); // Link
	      struct stat64 sbuf;
	      sbuf.st_mode = 0;
	      char filename[MAXPATHLEN + 1];
	      snprintf (filename, sizeof (filename), NTXT ("%s/%s"), path, entry->d_name);
	      dbe_stat (filename, &sbuf);
	      if (S_IREAD & sbuf.st_mode)
		{ // Readable
		  if (S_ISDIR (sbuf.st_mode) != 0)  // Directory
		    attr = NTXT ("/");
		  else if (S_ISREG (sbuf.st_mode) != 0) // Regular file
		    attr = NTXT ("");
		}
	      sb.append (attr);
	    }
	  sb.append (NTXT ("\n"));
	}
    }
  closedir (dir);
  return sb.toString ();
}

/**
 * Gets list of processes according to the specified format
 * Supported formats:
 * "/bin/ps -ef" - see 'man ps' for details
 * @param format
 * @return char * processes
 */
char *
dbe_get_processes (const char *format)
{
  StringBuilder sb;
  if (!strcmp (format, NTXT ("/bin/ps -ef")))
    {
      char buf[BUFSIZ];
      FILE *ptr = popen (format, "r");
      if (ptr != NULL)
	{
	  while (fgets (buf, BUFSIZ, ptr) != NULL)
	    sb.append (buf);
	  pclose (ptr);
	}
    }
  return sb.toString ();
}

/**
 * Creates the directory named by the specified path name, including any
 * necessary but nonexistent parent directories.
 * Uses system utility "/bin/mkdir -p"
 * Temporary limitation: path name should not contain spaces.
 * Returns message from "/bin/mkdir -p"
 * @param pathname
 * @return result
 */
char *
dbe_create_directories (const char *pathname)
{
  StringBuilder sb;
  char *makedir = dbe_sprintf (NTXT ("/bin/mkdir -p %s 2>&1"), pathname);
  char out[BUFSIZ];
  FILE *ptr = popen (makedir, "r");
  if (ptr != NULL)
    {
      while (fgets (out, BUFSIZ, ptr) != NULL)
	sb.append (out);
      pclose (ptr);
    }
  free (makedir);
  DIR *dir = opendir (pathname);
  if (dir != NULL)
    {
      closedir (dir);
      return NULL; // success
    }
  else
    sb.append (NTXT ("\nError: Cannot open directory\n")); // DEBUG
  return sb.toString (); // error
}

/**
 * Deletes the file or the directory named by the specified path name.
 * If this pathname denotes a directory, then the directory must be empty in order to be deleted.
 * Uses system utility "/bin/rm" or "/bin/rmdir"
 * Temporary limitation: path name should not contain spaces.
 * Returns error message from system utility
 * @param pathname
 * @return result
 */
char *
dbe_delete_file (const char *pathname)
{
  StringBuilder sb;
  char *cmd = NULL;
  struct stat64 sbuf;
  sbuf.st_mode = 0;
  int st = dbe_stat (pathname, &sbuf);
  if (st == 0)
    { // Exists
      if (S_ISDIR (sbuf.st_mode) != 0)      // Directory
	cmd = dbe_sprintf (NTXT ("/bin/rmdir %s 2>&1"), pathname);
      else if (S_ISREG (sbuf.st_mode) != 0)     // Regular file
	cmd = dbe_sprintf (NTXT ("/bin/rm %s 2>&1"), pathname);
    }
  else
    return NULL; // Nothing to remove
  if (cmd != NULL)
    {
      char out[BUFSIZ];
      FILE *ptr = popen (cmd, "r");
      if (ptr != NULL)
	{
	  while (fgets (out, BUFSIZ, ptr) != NULL)
	    sb.append (out);
	  pclose (ptr);
	}
      free (cmd);
    }
  else
    sb.sprintf (NTXT ("Error: cannot remove %s - not a regular file and not a directory\n"), pathname);
  return sb.toString ();
}

char *
dbe_xml2str (const char *s)
{
  if (s == NULL)
    return NULL;
  StringBuilder sb;
  while (*s)
    {
      if (*s == '&')
	{
	  if (strncmp (s, NTXT ("&nbsp;"), 6) == 0)
	    {
	      sb.append (' ');
	      s += 6;
	      continue;
	    }
	  else if (strncmp (s, NTXT ("&quot;"), 6) == 0)
	    {
	      sb.append ('"');
	      s += 6;
	      continue;
	    }
	  else if (strncmp (s, NTXT ("&amp;"), 5) == 0)
	    {
	      sb.append ('&');
	      s += 5;
	      continue;
	    }
	  else if (strncmp (s, NTXT ("&lt;"), 4) == 0)
	    {
	      sb.append ('<');
	      s += 4;
	      continue;
	    }
	  else if (strncmp (s, NTXT ("&gt;"), 4) == 0)
	    {
	      sb.append ('>');
	      s += 4;
	      continue;
	    }
	}
      sb.append (*s);
      s++;
    }
  return sb.toString ();
}

void
swapByteOrder (void *p, size_t sz)
{
  if (sz == 8)
    {
      uint64_t *pv = (uint64_t *) p;
      uint64_t v = *pv;
      v = ((v & 0x00000000FF000000) << 8) | ((v >> 8) & 0x00000000FF000000) |
	      ((v & 0x0000000000FF0000) << 24) | ((v >> 24) & 0x0000000000FF0000) |
	      ((v & 0x000000000000FF00) << 40) | ((v >> 40) & 0x000000000000FF00) |
	      (v >> 56) | (v << 56);
      *pv = v;
    }
  else if (sz == 4)
    {
      uint32_t *pv = (uint32_t *) p;
      uint32_t v = *pv;
      v = (v >> 24) | (v << 24) | ((v & 0x0000FF00) << 8) | ((v >> 8) & 0x0000FF00);
      *pv = v;
    }
  else if (sz == 2)
    {
      uint16_t *pv = (uint16_t *) p;
      uint16_t v = *pv;
      v = (v >> 8) | (v << 8);
      *pv = v;
    }
}

void
destroy (void *vec)
{
  if (vec == NULL)
    return;
  Vector<void*> *array = (Vector<void*>*)vec;
  switch (array->type ())
    {
    case VEC_STRING:
      ((Vector<char *>*)array)->destroy ();
      break;
    case VEC_VOIDARR:
    case VEC_STRINGARR:
    case VEC_INTARR:
    case VEC_BOOLARR:
    case VEC_LLONGARR:
    case VEC_DOUBLEARR:
      for (long i = 0; i < array->size (); i++)
	destroy (array->fetch (i));
      break;
    case VEC_INTEGER:
    case VEC_CHAR:
    case VEC_BOOL:
    case VEC_DOUBLE:
    case VEC_LLONG:
    default:
      break;
    }
  delete array;
}

int64_t
read_from_file (int fd, void *buffer, int64_t nbyte)
{
  int64_t cnt = 0;
  char *buf = (char *) buffer;
  while (nbyte > 0)
    { // Sometimes system cannot read 'nbyte'
      ssize_t n = read (fd, (void *) (buf + cnt), (size_t) nbyte);
      if (n <= 0)
	break;
      nbyte -= n;
      cnt += n;
    }
  return cnt;
}

/**
 * Create symbolic link to the path
 * @param  path - path with spaces
 * @param  dir  - directory where the link should be created
 * @return symbolic link
 */
char *
dbe_create_symlink_to_path (const char *path, const char *dir)
{
  char *symbolic_link = NULL;
  if (NULL == path || NULL == dir)
    return NULL;
  int res = mkdir (dir, 0777);
  if (res != 0 && dbe_stat (dir, NULL) != 0)
    return NULL; // Cannot create directory
  long len = dbe_sstrlen (path);
  if (len <= 4)
    return NULL; // Unknown situation
  if (strcmp ((path + len - 4), "/bin") != 0)   // Unknown situation
    return NULL;
  int max = 99; // Just an arbitrary number
  for (int i = 1; i <= max; i++)
    {
      // Try to create symbolic link
      char *d = dbe_sprintf ("%s/%d", dir, i);
      if (NULL == d)
	return NULL;
      res = mkdir (d, 0777);
      symbolic_link = dbe_sprintf ("%s/%s", d, "bin");
      free (d);
      if (NULL == symbolic_link) // Not enough memory
	return NULL;
      res = symlink (path, symbolic_link);
      if (res == 0)     // Link is created - use it.
	break;
      // Check if such link already exists
      int e = errno;
      char buf[MAXPATHLEN + 1];
      memset (buf, 0, MAXPATHLEN + 1);
      ssize_t n = readlink (symbolic_link, buf, MAXPATHLEN);
      if (n == len && strcmp (path, buf) == 0) // Link is correct - use it.
	break;
      if (i == max)
	{ // report the error
	  fprintf (stderr, GTXT ("Error: symlink(%s, %s) returned error: %d\n"), path, symbolic_link, res);
	  fprintf (stderr, GTXT ("Error: errno=%d (%s)\n"), e, strerror (e));
	  fflush (stderr);
	}
      free (symbolic_link);
      symbolic_link = NULL;
    }
  return symbolic_link;
}

// Compute checksum for specified file.
// This code is from usr/src/cmd/cksum.c, adapted for us
// crcposix -- compute posix.2 compatable 32 bit CRC
//
// The POSIX.2 (draft 10) CRC algorithm.
// This is a 32 bit CRC with polynomial
//      x**32 + x**26 + x**23 + x**22 + x**16 + x**12 + x**11 + x**10 +
//      x**8  + x**7  + x**5  + x**4  + x**2  + x**1  + x**0
//
// layout is from the POSIX.2 Rationale

static uint32_t crctab_posix[256] = {
  0x00000000L,
  0x04C11DB7L, 0x09823B6EL, 0x0D4326D9L, 0x130476DCL, 0x17C56B6BL,
  0x1A864DB2L, 0x1E475005L, 0x2608EDB8L, 0x22C9F00FL, 0x2F8AD6D6L,
  0x2B4BCB61L, 0x350C9B64L, 0x31CD86D3L, 0x3C8EA00AL, 0x384FBDBDL,
  0x4C11DB70L, 0x48D0C6C7L, 0x4593E01EL, 0x4152FDA9L, 0x5F15ADACL,
  0x5BD4B01BL, 0x569796C2L, 0x52568B75L, 0x6A1936C8L, 0x6ED82B7FL,
  0x639B0DA6L, 0x675A1011L, 0x791D4014L, 0x7DDC5DA3L, 0x709F7B7AL,
  0x745E66CDL, 0x9823B6E0L, 0x9CE2AB57L, 0x91A18D8EL, 0x95609039L,
  0x8B27C03CL, 0x8FE6DD8BL, 0x82A5FB52L, 0x8664E6E5L, 0xBE2B5B58L,
  0xBAEA46EFL, 0xB7A96036L, 0xB3687D81L, 0xAD2F2D84L, 0xA9EE3033L,
  0xA4AD16EAL, 0xA06C0B5DL, 0xD4326D90L, 0xD0F37027L, 0xDDB056FEL,
  0xD9714B49L, 0xC7361B4CL, 0xC3F706FBL, 0xCEB42022L, 0xCA753D95L,
  0xF23A8028L, 0xF6FB9D9FL, 0xFBB8BB46L, 0xFF79A6F1L, 0xE13EF6F4L,
  0xE5FFEB43L, 0xE8BCCD9AL, 0xEC7DD02DL, 0x34867077L, 0x30476DC0L,
  0x3D044B19L, 0x39C556AEL, 0x278206ABL, 0x23431B1CL, 0x2E003DC5L,
  0x2AC12072L, 0x128E9DCFL, 0x164F8078L, 0x1B0CA6A1L, 0x1FCDBB16L,
  0x018AEB13L, 0x054BF6A4L, 0x0808D07DL, 0x0CC9CDCAL, 0x7897AB07L,
  0x7C56B6B0L, 0x71159069L, 0x75D48DDEL, 0x6B93DDDBL, 0x6F52C06CL,
  0x6211E6B5L, 0x66D0FB02L, 0x5E9F46BFL, 0x5A5E5B08L, 0x571D7DD1L,
  0x53DC6066L, 0x4D9B3063L, 0x495A2DD4L, 0x44190B0DL, 0x40D816BAL,
  0xACA5C697L, 0xA864DB20L, 0xA527FDF9L, 0xA1E6E04EL, 0xBFA1B04BL,
  0xBB60ADFCL, 0xB6238B25L, 0xB2E29692L, 0x8AAD2B2FL, 0x8E6C3698L,
  0x832F1041L, 0x87EE0DF6L, 0x99A95DF3L, 0x9D684044L, 0x902B669DL,
  0x94EA7B2AL, 0xE0B41DE7L, 0xE4750050L, 0xE9362689L, 0xEDF73B3EL,
  0xF3B06B3BL, 0xF771768CL, 0xFA325055L, 0xFEF34DE2L, 0xC6BCF05FL,
  0xC27DEDE8L, 0xCF3ECB31L, 0xCBFFD686L, 0xD5B88683L, 0xD1799B34L,
  0xDC3ABDEDL, 0xD8FBA05AL, 0x690CE0EEL, 0x6DCDFD59L, 0x608EDB80L,
  0x644FC637L, 0x7A089632L, 0x7EC98B85L, 0x738AAD5CL, 0x774BB0EBL,
  0x4F040D56L, 0x4BC510E1L, 0x46863638L, 0x42472B8FL, 0x5C007B8AL,
  0x58C1663DL, 0x558240E4L, 0x51435D53L, 0x251D3B9EL, 0x21DC2629L,
  0x2C9F00F0L, 0x285E1D47L, 0x36194D42L, 0x32D850F5L, 0x3F9B762CL,
  0x3B5A6B9BL, 0x0315D626L, 0x07D4CB91L, 0x0A97ED48L, 0x0E56F0FFL,
  0x1011A0FAL, 0x14D0BD4DL, 0x19939B94L, 0x1D528623L, 0xF12F560EL,
  0xF5EE4BB9L, 0xF8AD6D60L, 0xFC6C70D7L, 0xE22B20D2L, 0xE6EA3D65L,
  0xEBA91BBCL, 0xEF68060BL, 0xD727BBB6L, 0xD3E6A601L, 0xDEA580D8L,
  0xDA649D6FL, 0xC423CD6AL, 0xC0E2D0DDL, 0xCDA1F604L, 0xC960EBB3L,
  0xBD3E8D7EL, 0xB9FF90C9L, 0xB4BCB610L, 0xB07DABA7L, 0xAE3AFBA2L,
  0xAAFBE615L, 0xA7B8C0CCL, 0xA379DD7BL, 0x9B3660C6L, 0x9FF77D71L,
  0x92B45BA8L, 0x9675461FL, 0x8832161AL, 0x8CF30BADL, 0x81B02D74L,
  0x857130C3L, 0x5D8A9099L, 0x594B8D2EL, 0x5408ABF7L, 0x50C9B640L,
  0x4E8EE645L, 0x4A4FFBF2L, 0x470CDD2BL, 0x43CDC09CL, 0x7B827D21L,
  0x7F436096L, 0x7200464FL, 0x76C15BF8L, 0x68860BFDL, 0x6C47164AL,
  0x61043093L, 0x65C52D24L, 0x119B4BE9L, 0x155A565EL, 0x18197087L,
  0x1CD86D30L, 0x029F3D35L, 0x065E2082L, 0x0B1D065BL, 0x0FDC1BECL,
  0x3793A651L, 0x3352BBE6L, 0x3E119D3FL, 0x3AD08088L, 0x2497D08DL,
  0x2056CD3AL, 0x2D15EBE3L, 0x29D4F654L, 0xC5A92679L, 0xC1683BCEL,
  0xCC2B1D17L, 0xC8EA00A0L, 0xD6AD50A5L, 0xD26C4D12L, 0xDF2F6BCBL,
  0xDBEE767CL, 0xE3A1CBC1L, 0xE760D676L, 0xEA23F0AFL, 0xEEE2ED18L,
  0xF0A5BD1DL, 0xF464A0AAL, 0xF9278673L, 0xFDE69BC4L, 0x89B8FD09L,
  0x8D79E0BEL, 0x803AC667L, 0x84FBDBD0L, 0x9ABC8BD5L, 0x9E7D9662L,
  0x933EB0BBL, 0x97FFAD0CL, 0xAFB010B1L, 0xAB710D06L, 0xA6322BDFL,
  0xA2F33668L, 0xBCB4666DL, 0xB8757BDAL, 0xB5365D03L, 0xB1F740B4L
};

static void
m_crcposix (uint32_t *crcp, unsigned char *bp, uint32_t n)
{
  while (n-- > 0)
    *crcp = (*crcp << 8) ^ crctab_posix[(unsigned char) ((*crcp >> 24)^*bp++)];
}

// Do CRC-POSIX function by calling a library entry point that has a
// slightly different calling sequence.
static uint32_t
docrcposix (uint32_t crcval, unsigned char *bp, uint32_t n)
{
  m_crcposix (&crcval, bp, n);
  return (crcval);
}

// Sum algorithms require various kinds of post-processing.
// The 'S' and 'R' variables are from the POSIX.2 (Draft 8?) description
// of the "sum" utility.
static uint32_t
postprocess (uint32_t S, long long n)
{
  // POSIX tacks on significant bytes of the length so that
  // different length sequences of '\0' have different sums;
  // then it complements sum.
  unsigned char char_n[sizeof (n)];
  uint32_t i;
  for (i = 0; n != 0; n >>= 8, ++i)
    char_n[i] = (unsigned char) (n & 0xFF);
  return (~docrcposix (S, char_n, i));
}

uint32_t
get_cksum (const char * pathname, char ** errmsg)
{
  int fd = open (pathname, O_RDONLY);
  if (fd < 0)
    {
      if (errmsg)
	*errmsg = dbe_sprintf (GTXT ("*** Warning: Error opening file for reading: %s"), pathname);
      return 0; // error
    }
  uint32_t crcval = 0;
  long long bytes = 0;
  int64_t n;
  unsigned char buf[4096];
  while ((n = read_from_file (fd, (char *) buf, sizeof (buf))) > 0)
    {
      bytes += n;
      crcval = docrcposix (crcval, buf, n);
    }
  close (fd);
  crcval = postprocess (crcval, bytes);
  return crcval;
}
