/* strings -- print the strings of printable characters in files
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Usage: strings [options] file...

   Options:
   --all
   -a
   -		Scan each file in its entirety.

   --data
   -d		Scan only the initialized data section(s) of object files.

   --print-file-name
   -f		Print the name of the file before each string.

   --bytes=min-len
   -n min-len
   -min-len	Print graphic char sequences, MIN-LEN or more bytes long,
		that are followed by a NUL or a non-displayable character.
		Default is 4.

   --radix={o,x,d}
   -t {o,x,d}	Print the offset within the file before each string,
		in octal/hex/decimal.

  --include-all-whitespace
  -w		By default tab and space are the only whitepace included in graphic
		char sequences.  This option considers all of isspace() valid.

   -o		Like -to.  (Some other implementations have -o like -to,
		others like -td.  We chose one arbitrarily.)

   --encoding={s,S,b,l,B,L}
   -e {s,S,b,l,B,L}
		Select character encoding: 7-bit-character, 8-bit-character,
		bigendian 16-bit, littleendian 16-bit, bigendian 32-bit,
		littleendian 32-bit.

   --target=BFDNAME
   -T {bfdname}
		Specify a non-default object file format.

  --unicode={default|locale|invalid|hex|escape|highlight}
  -U {d|l|i|x|e|h}
		Determine how to handle UTF-8 unicode characters.  The default
		is no special treatment.  All other versions of this option
		only apply if the encoding is valid and enabling the option
		implies --encoding=S.
		The 'locale' option displays the characters according to the
		current locale.  The 'invalid' option treats them as
		non-string characters.  The 'hex' option displays them as hex
		byte sequences.  The 'escape' option displays them as escape
		sequences and the 'highlight' option displays them as
		coloured escape sequences.

  --output-separator=sep_string
  -s sep_string	String used to separate parsed strings in output.
		Default is newline.

   --help
   -h		Print the usage message on the standard output.

   --version
   -V
   -v		Print the program version number.

   Written by Richard Stallman <rms@gnu.ai.mit.edu>
   and David MacKenzie <djm@gnu.ai.mit.edu>.  */

#include "sysdep.h"
#include "bfd.h"
#include "getopt.h"
#include "libiberty.h"
#include "safe-ctype.h"
#include "bucomm.h"

#ifndef streq
#define streq(a,b) (strcmp ((a),(b)) == 0)
#endif

typedef enum unicode_display_type
{
  unicode_default = 0,
  unicode_locale,
  unicode_escape,
  unicode_hex,
  unicode_highlight,
  unicode_invalid
} unicode_display_type;

static unicode_display_type unicode_display = unicode_default;

#define STRING_ISGRAPHIC(c) \
      (   (c) >= 0 \
       && (c) <= 255 \
       && ((c) == '\t' || ISPRINT (c) || (encoding == 'S' && (c) > 127) \
	   || (include_all_whitespace && ISSPACE (c))) \
      )

#ifndef errno
extern int errno;
#endif

/* The BFD section flags that identify an initialized data section.  */
#define DATA_FLAGS (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS)

/* Radix for printing addresses (must be 8, 10 or 16).  */
static int address_radix;

/* Minimum length of sequence of graphic chars to trigger output.  */
static unsigned int string_min;

/* Whether or not we include all whitespace as a graphic char.   */
static bool include_all_whitespace;

/* TRUE means print address within file for each string.  */
static bool print_addresses;

/* TRUE means print filename for each string.  */
static bool print_filenames;

/* TRUE means for object files scan only the data section.  */
static bool datasection_only;

/* The BFD object file format.  */
static char *target;

/* The character encoding format.  */
static char encoding;
static int encoding_bytes;

/* Output string used to separate parsed strings  */
static char *output_separator;

static struct option long_options[] =
{
  {"all", no_argument, NULL, 'a'},
  {"bytes", required_argument, NULL, 'n'},
  {"data", no_argument, NULL, 'd'},
  {"encoding", required_argument, NULL, 'e'},
  {"help", no_argument, NULL, 'h'},
  {"include-all-whitespace", no_argument, NULL, 'w'},
  {"output-separator", required_argument, NULL, 's'},
  {"print-file-name", no_argument, NULL, 'f'},
  {"radix", required_argument, NULL, 't'},
  {"target", required_argument, NULL, 'T'},
  {"unicode", required_argument, NULL, 'U'},
  {"version", no_argument, NULL, 'v'},
  {NULL, 0, NULL, 0}
};

static bool strings_file (char *);
static void print_strings (const char *, FILE *, file_ptr, int, char *);
static void usage (FILE *, int) ATTRIBUTE_NORETURN;

int main (int, char **);

static void
set_string_min (const char * arg)
{
  char *s;
  unsigned long l = strtoul (arg, &s, 0);

  if (s != NULL && *s != 0)
    fatal (_("invalid integer argument %s"), arg);

  string_min = (unsigned int) l;

  if (l != (unsigned long) string_min)
    fatal (_("minimum string length is too big: %s"), arg);
    
  if (string_min < 1)
    fatal (_("minimum string length is too small: %s"), arg);

  /* PR 30595: Look for minimum string lengths that overflow an 'int'.  */
  if (string_min + 1 == 0)
    fatal (_("minimum string length %s is too big"), arg);

  /* FIXME: Should we warn for unreasonably large minimum
     string lengths, even if technically they will work ?  */
}

int
main (int argc, char **argv)
{
  int optc;
  int exit_status = 0;
  bool files_given = false;
  int numeric_opt = 0;

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = argv[0];
  xmalloc_set_program_name (program_name);
  bfd_set_error_program_name (program_name);

  expandargv (&argc, &argv);

  string_min = 4;
  include_all_whitespace = false;
  print_addresses = false;
  print_filenames = false;
  if (DEFAULT_STRINGS_ALL)
    datasection_only = false;
  else
    datasection_only = true;
  target = NULL;
  encoding = 's';
  output_separator = NULL;

  while ((optc = getopt_long (argc, argv, "adfhHn:wot:e:T:s:U:Vv0123456789",
			      long_options, (int *) 0)) != EOF)
    {
      switch (optc)
	{
	case 'a':
	  datasection_only = false;
	  break;

	case 'd':
	  datasection_only = true;
	  break;

	case 'f':
	  print_filenames = true;
	  break;

	case 'H':
	case 'h':
	  usage (stdout, 0);

	case 'n':
	  set_string_min (optarg);
	  break;

	case 'w':
	  include_all_whitespace = true;
	  break;

	case 'o':
	  print_addresses = true;
	  address_radix = 8;
	  break;

	case 't':
	  print_addresses = true;
	  if (optarg[1] != '\0')
	    usage (stderr, 1);
	  switch (optarg[0])
	    {
	    case 'o':
	      address_radix = 8;
	      break;

	    case 'd':
	      address_radix = 10;
	      break;

	    case 'x':
	      address_radix = 16;
	      break;

	    default:
	      usage (stderr, 1);
	    }
	  break;

	case 'T':
	  target = optarg;
	  break;

	case 'e':
	  if (optarg[1] != '\0')
	    usage (stderr, 1);
	  encoding = optarg[0];
	  break;

	case 's':
	  output_separator = optarg;
	  break;

	case 'U':
	  if (streq (optarg, "default") || streq (optarg, "d"))
	    unicode_display = unicode_default;
	  else if (streq (optarg, "locale") || streq (optarg, "l"))
	    unicode_display = unicode_locale;
	  else if (streq (optarg, "escape") || streq (optarg, "e"))
	    unicode_display = unicode_escape;
	  else if (streq (optarg, "invalid") || streq (optarg, "i"))
	    unicode_display = unicode_invalid;
	  else if (streq (optarg, "hex") || streq (optarg, "x"))
	    unicode_display = unicode_hex;
	  else if (streq (optarg, "highlight") || streq (optarg, "h"))
	    unicode_display = unicode_highlight;
	  else
	    fatal (_("invalid argument to -U/--unicode: %s"), optarg);
	  break;

	case 'V':
	case 'v':
	  print_version ("strings");
	  break;

	case '?':
	  usage (stderr, 1);

	default:
	  numeric_opt = optind;
	  break;
	}
    }

  if (unicode_display != unicode_default)
    encoding = 'S';

  if (numeric_opt != 0)
    set_string_min (argv[numeric_opt - 1] + 1);

  switch (encoding)
    {
    case 'S':
    case 's':
      encoding_bytes = 1;
      break;
    case 'b':
    case 'l':
      encoding_bytes = 2;
      break;
    case 'B':
    case 'L':
      encoding_bytes = 4;
      break;
    default:
      usage (stderr, 1);
    }

  if (bfd_init () != BFD_INIT_MAGIC)
    fatal (_("fatal error: libbfd ABI mismatch"));
  set_default_bfd_target ();

  if (optind >= argc)
    {
      datasection_only = false;
      SET_BINARY (fileno (stdin));
      print_strings ("{standard input}", stdin, 0, 0, (char *) NULL);
      files_given = true;
    }
  else
    {
      for (; optind < argc; ++optind)
	{
	  if (streq (argv[optind], "-"))
	    datasection_only = false;
	  else
	    {
	      files_given = true;
	      exit_status |= !strings_file (argv[optind]);
	    }
	}
    }

  if (!files_given)
    usage (stderr, 1);

  return (exit_status);
}

/* Scan section SECT of the file ABFD, whose printable name is
   FILENAME.  If it contains initialized data set GOT_A_SECTION and
   print the strings in it.  */

static void
strings_a_section (bfd *abfd, asection *sect, const char *filename,
		   bool *got_a_section)
{
  bfd_size_type sectsize;
  bfd_byte *mem;

  if ((sect->flags & DATA_FLAGS) != DATA_FLAGS)
    return;

  sectsize = bfd_section_size (sect);
  if (sectsize == 0)
    return;

  if (!bfd_malloc_and_get_section (abfd, sect, &mem))
    {
      non_fatal (_("%s: Reading section %s failed: %s"),
		 filename, sect->name, bfd_errmsg (bfd_get_error ()));
      return;
    }

  *got_a_section = true;
  print_strings (filename, NULL, sect->filepos, sectsize, (char *) mem);
  free (mem);
}

/* Scan all of the sections in FILE, and print the strings
   in the initialized data section(s).

   Return TRUE if successful,
   FALSE if not (such as if FILE is not an object file).  */

static bool
strings_object_file (const char *file)
{
  bfd *abfd;
  asection *s;
  bool got_a_section;

  abfd = bfd_openr (file, target);

  if (abfd == NULL)
    /* Treat the file as a non-object file.  */
    return false;

  /* This call is mainly for its side effect of reading in the sections.
     We follow the traditional behavior of `strings' in that we don't
     complain if we don't recognize a file to be an object file.  */
  if (!bfd_check_format (abfd, bfd_object))
    {
      bfd_close (abfd);
      return false;
    }

  got_a_section = false;
  for (s = abfd->sections; s != NULL; s = s->next)
    strings_a_section (abfd, s, file, &got_a_section);

  if (!bfd_close (abfd))
    {
      bfd_nonfatal (file);
      return false;
    }

  return got_a_section;
}

/* Print the strings in FILE.  Return TRUE if ok, FALSE if an error occurs.  */

static bool
strings_file (char *file)
{
  struct stat st;

  /* get_file_size does not support non-S_ISREG files.  */

  if (stat (file, &st) < 0)
    {
      if (errno == ENOENT)
	non_fatal (_("'%s': No such file"), file);
      else
	non_fatal (_("Warning: could not locate '%s'.  reason: %s"),
		   file, strerror (errno));
      return false;
    }
  else if (S_ISDIR (st.st_mode))
    {
      non_fatal (_("Warning: '%s' is a directory"), file);
      return false;
    }

  /* If we weren't told to scan the whole file,
     try to open it as an object file and only look at
     initialized data sections.  If that fails, fall back to the
     whole file.  */
  if (!datasection_only || !strings_object_file (file))
    {
      FILE *stream;

      stream = fopen (file, FOPEN_RB);
      if (stream == NULL)
	{
	  fprintf (stderr, "%s: ", program_name);
	  perror (file);
	  return false;
	}

      print_strings (file, stream, (file_ptr) 0, 0, (char *) NULL);

      if (fclose (stream) == EOF)
	{
	  fprintf (stderr, "%s: ", program_name);
	  perror (file);
	  return false;
	}
    }

  return true;
}

/* Read the next character, return EOF if none available.
   Assume that STREAM is positioned so that the next byte read
   is at address ADDRESS in the file.

   If STREAM is NULL, do not read from it.
   The caller can supply a buffer of characters
   to be processed before the data in STREAM.
   MAGIC is the address of the buffer and
   MAGICCOUNT is how many characters are in it.  */

static long
get_char (FILE *stream, file_ptr *address, int *magiccount, char **magic)
{
  int c, i;
  long r = 0;

  for (i = 0; i < encoding_bytes; i++)
    {
      if (*magiccount)
	{
	  (*magiccount)--;
	  c = *(*magic)++;
	}
      else
	{
	  if (stream == NULL)
	    return EOF;

	  /* Only use getc_unlocked if we found a declaration for it.
	     Otherwise, libc is not thread safe by default, and we
	     should not use it.  */

#if defined(HAVE_GETC_UNLOCKED) && HAVE_DECL_GETC_UNLOCKED
	  c = getc_unlocked (stream);
#else
	  c = getc (stream);
#endif
	  if (c == EOF)
	    return EOF;
	}

      (*address)++;
      r = (r << 8) | (c & 0xff);
    }

  switch (encoding)
    {
    default:
      break;
    case 'l':
      r = ((r & 0xff) << 8) | ((r & 0xff00) >> 8);
      break;
    case 'L':
      r = (((r & 0xff) << 24) | ((r & 0xff00) << 8)
	   | ((r & 0xff0000) >> 8) | ((r & 0xff000000) >> 24));
      break;
    }

  return r;
}

/* Throw away one byte of a (possibly) multi-byte char C, updating
   address and buffer to suit.  */

static void
unget_part_char (long c, file_ptr *address, int *magiccount, char **magic)
{
  static char tmp[4];

  if (encoding_bytes > 1)
    {
      *address -= encoding_bytes - 1;

      if (*magiccount == 0)
	{
	  /* If no magic buffer exists, use temp buffer.  */
	  switch (encoding)
	    {
	    default:
	      break;
	    case 'b':
	      tmp[0] = c & 0xff;
	      *magiccount = 1;
	      break;
	    case 'l':
	      tmp[0] = (c >> 8) & 0xff;
	      *magiccount = 1;
	      break;
	    case 'B':
	      tmp[0] = (c >> 16) & 0xff;
	      tmp[1] = (c >> 8) & 0xff;
	      tmp[2] = c & 0xff;
	      *magiccount = 3;
	      break;
	    case 'L':
	      tmp[0] = (c >> 8) & 0xff;
	      tmp[1] = (c >> 16) & 0xff;
	      tmp[2] = (c >> 24) & 0xff;
	      *magiccount = 3;
	      break;
	    }
	  *magic = tmp;
	}
      else
	{
	  /* If magic buffer exists, rewind.  */
	  *magic -= encoding_bytes - 1;
	  *magiccount += encoding_bytes - 1;
	}
    }
}

static void
print_filename_and_address (const char * filename, file_ptr address)
{
  if (print_filenames)
    printf ("%s: ", filename);

  if (! print_addresses)
    return;

  switch (address_radix)
    {
    case 8:
      if (sizeof (address) > sizeof (long))
	{
#ifndef __MSVCRT__
	  printf ("%7llo ", (unsigned long long) address);
#else
	  printf ("%7I64o ", (unsigned long long) address);
#endif
	}
      else
	printf ("%7lo ", (unsigned long) address);
      break;

    case 10:
      if (sizeof (address) > sizeof (long))
	{
#ifndef __MSVCRT__
	  printf ("%7llu ", (unsigned long long) address);
#else
	  printf ("%7I64d ", (unsigned long long) address);
#endif
	}
      else
	printf ("%7ld ", (long) address);
      break;

    case 16:
      if (sizeof (address) > sizeof (long))
	{
#ifndef __MSVCRT__
	  printf ("%7llx ", (unsigned long long) address);
#else
	  printf ("%7I64x ", (unsigned long long) address);
#endif
	}
      else
	printf ("%7lx ", (unsigned long) address);
      break;
    }
}

/* Return non-zero if the bytes starting at BUFFER form a valid UTF-8 encoding.
   If the encoding is valid then returns the number of bytes it uses.  */

static unsigned int
is_valid_utf8 (const unsigned char * buffer, unsigned long buflen)
{
  if (buffer[0] < 0xc0)
    return 0;

  if (buflen < 2)
    return 0;

  if ((buffer[1] & 0xc0) != 0x80)
    return 0;

  if ((buffer[0] & 0x20) == 0)
    return 2;

  if (buflen < 3)
    return 0;

  if ((buffer[2] & 0xc0) != 0x80)
    return 0;

  if ((buffer[0] & 0x10) == 0)
    return 3;

  if (buflen < 4)
    return 0;

  if ((buffer[3] & 0xc0) != 0x80)
    return 0;

  return 4;
}

/* Display a UTF-8 encoded character in BUFFER according to the setting
   of unicode_display.  The character is known to be valid.
   Returns the number of bytes consumed.  */

static unsigned int
display_utf8_char (const unsigned char * buffer)
{
  unsigned int j;
  unsigned int utf8_len;

  switch (buffer[0] & 0x30)
    {
    case 0x00:
    case 0x10:
      utf8_len = 2;
      break;
    case 0x20:
      utf8_len = 3;
      break;
    default:
      utf8_len = 4;
    }

  switch (unicode_display)
    {
    default:
      fprintf (stderr, "ICE: unexpected unicode display type\n");
      break;

    case unicode_escape:
    case unicode_highlight:
      if (unicode_display == unicode_highlight && isatty (1))
	printf ("\x1B[31;47m"); /* Red.  */

      switch (utf8_len)
	{
	case 2:
	  printf ("\\u%02x%02x",
		  ((buffer[0] & 0x1c) >> 2),
		  ((buffer[0] & 0x03) << 6) | (buffer[1] & 0x3f));
	  break;

	case 3:
	  printf ("\\u%02x%02x",
		  ((buffer[0] & 0x0f) << 4) | ((buffer[1] & 0x3c) >> 2),
		  ((buffer[1] & 0x03) << 6) | ((buffer[2] & 0x3f)));
	  break;

	case 4:
	  printf ("\\u%02x%02x%02x",
		  ((buffer[0] & 0x07) << 6) | ((buffer[1] & 0x3c) >> 2),
		  ((buffer[1] & 0x03) << 6) | ((buffer[2] & 0x3c) >> 2),
		  ((buffer[2] & 0x03) << 6) | ((buffer[3] & 0x3f)));
	  break;
	default:
	  /* URG.  */
	  break;
	}

      if (unicode_display == unicode_highlight && isatty (1))
	printf ("\033[0m"); /* Default colour.  */
      break;

    case unicode_hex:
      putchar ('<');
      printf ("0x");
      for (j = 0; j < utf8_len; j++)
	printf ("%02x", buffer [j]);
      putchar ('>');
      break;

    case unicode_locale:
      printf ("%.1s", buffer);
      break;
    }

  return utf8_len;
}

/* Display strings in BUFFER.  Treat any UTF-8 encoded characters encountered
   according to the setting of the unicode_display variable.  The buffer
   contains BUFLEN bytes.

   Display the characters as if they started at ADDRESS and are contained in
   FILENAME.  */

static void
print_unicode_buffer (const char *            filename,
		      file_ptr                address,
		      const unsigned char *   buffer,
		      unsigned long           buflen)
{
  /* Paranoia checks...  */
  if (filename == NULL
      || buffer == NULL
      || unicode_display == unicode_default
      || encoding != 'S'
      || encoding_bytes != 1)
    {
      fprintf (stderr, "ICE: bad arguments to print_unicode_buffer\n");
      return;
    }

  if (buflen == 0)
    return;

  /* We must only display strings that are at least string_min *characters*
     long.  So we scan the buffer in two stages.  First we locate the start
     of a potential string.  Then we walk along it until we have found
     string_min characters.  Then we go back to the start point and start
     displaying characters according to the unicode_display setting.  */

  unsigned long start_point = 0;
  unsigned long i = 0;
  unsigned int char_len = 1;
  unsigned int num_found = 0;

  for (i = 0; i < buflen; i += char_len)
    {
      int c = buffer[i];

      char_len = 1;

      /* Find the first potential character of a string.  */
      if (! STRING_ISGRAPHIC (c))
	{
	  num_found = 0;
	  continue;
	}

      if (c > 126)
	{
	  if (c < 0xc0)
	    {
	      num_found = 0;
	      continue;
	    }

	  if ((char_len = is_valid_utf8 (buffer + i, buflen - i)) == 0)
	    {
	      char_len = 1;
	      num_found = 0;
	      continue;
	    }

	  if (unicode_display == unicode_invalid)
	    {
	      /* We have found a valid UTF-8 character, but we treat it as non-graphic.  */
	      num_found = 0;
	      continue;
	    }
	}

      if (num_found == 0)
	/* We have found a potential starting point for a string.  */
	start_point = i;

      ++ num_found;

      if (num_found >= string_min)
	break;
    }

  if (num_found < string_min)
    return;

  print_filename_and_address (filename, address + start_point);

  /* We have found string_min characters.  Display them and any
     more that follow.  */
  for (i = start_point; i < buflen; i += char_len)
    {
      int c = buffer[i];

      char_len = 1;

      if (! STRING_ISGRAPHIC (c))
	break;
      else if (c < 127)
	putchar (c);
      else if (! is_valid_utf8 (buffer + i, buflen - i))
	break;
      else if (unicode_display == unicode_invalid)
	break;
      else
	char_len = display_utf8_char (buffer + i);
    }

  if (output_separator)
    fputs (output_separator, stdout);
  else
    putchar ('\n');

  /* FIXME: Using tail recursion here is lazy programming...  */
  print_unicode_buffer (filename, address + i, buffer + i, buflen - i);
}

static int
get_unicode_byte (FILE *          stream,
		  unsigned char * putback,
		  unsigned int *  num_putback,
		  unsigned int *  num_read)
{
  if (* num_putback > 0)
    {
      * num_putback = * num_putback - 1;
      return putback [* num_putback];
    }

  * num_read = * num_read + 1;

#if defined(HAVE_GETC_UNLOCKED) && HAVE_DECL_GETC_UNLOCKED
  return getc_unlocked (stream);
#else
  return getc (stream);
#endif
}

/* Helper function for print_unicode_stream.  */

static void
print_unicode_stream_body (const char *     filename,
			   file_ptr         address,
			   FILE *           stream,
			   unsigned char *  putback_buf,
			   unsigned int     num_putback,
			   unsigned char *  print_buf)
{
  /* It would be nice if we could just read the stream into a buffer
     and then process if with print_unicode_buffer.  But the input
     might be huge or it might time-locked (eg stdin).  So instead
     we go one byte at a time...  */

  file_ptr start_point = 0;
  unsigned int num_read = 0;
  unsigned int num_chars = 0;
  unsigned int num_print = 0;
  int c = 0;

  /* Find a series of string_min characters.  Put them into print_buf.  */
  do
    {
      if (num_chars >= string_min)
	break;

      c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
      if (c == EOF)
	break;

      if (! STRING_ISGRAPHIC (c))
	{
	  num_chars = num_print = 0;
	  continue;
	}

      if (num_chars == 0)
	start_point = num_read - 1;

      if (c < 127)
	{
	  print_buf[num_print] = c;
	  num_chars ++;
	  num_print ++;
	  continue;
	}

      if (c < 0xc0)
	{
	  num_chars = num_print = 0;
	  continue;
	}

      /* We *might* have a UTF-8 sequence.  Time to start peeking.  */
      char utf8[4];

      utf8[0] = c;
      c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
      if (c == EOF)
	break;
      utf8[1] = c;

      if ((utf8[1] & 0xc0) != 0x80)
	{
	  /* Invalid UTF-8.  */
	  putback_buf[num_putback++] = utf8[1];
	  num_chars = num_print = 0;
	  continue;
	}
      else if ((utf8[0] & 0x20) == 0)
	{
	  /* A valid 2-byte UTF-8 encoding.  */
	  if (unicode_display == unicode_invalid)
	    {
	      putback_buf[num_putback++] = utf8[1];
	      num_chars = num_print = 0;
	    }
	  else
	    {
	      print_buf[num_print ++] = utf8[0];
	      print_buf[num_print ++] = utf8[1];
	      num_chars ++;
	    }
	  continue;
	}

      c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
      if (c == EOF)
	break;
      utf8[2] = c;

      if ((utf8[2] & 0xc0) != 0x80)
	{
	  /* Invalid UTF-8.  */
	  putback_buf[num_putback++] = utf8[2];
	  putback_buf[num_putback++] = utf8[1];
	  num_chars = num_print = 0;
	  continue;
	}
      else if ((utf8[0] & 0x10) == 0)
	{
	  /* A valid 3-byte UTF-8 encoding.  */
	  if (unicode_display == unicode_invalid)
	    {
	      putback_buf[num_putback++] = utf8[2];
	      putback_buf[num_putback++] = utf8[1];
	      num_chars = num_print = 0;
	    }
	  else
	    {
	      print_buf[num_print ++] = utf8[0];
	      print_buf[num_print ++] = utf8[1];
	      print_buf[num_print ++] = utf8[2];
	      num_chars ++;
	    }
	  continue;
	}

      c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
      if (c == EOF)
	break;
      utf8[3] = c;

      if ((utf8[3] & 0xc0) != 0x80)
	{
	  /* Invalid UTF-8.  */
	  putback_buf[num_putback++] = utf8[3];
	  putback_buf[num_putback++] = utf8[2];
	  putback_buf[num_putback++] = utf8[1];
	  num_chars = num_print = 0;
	}
      /* We have a valid 4-byte UTF-8 encoding.  */
      else if (unicode_display == unicode_invalid)
	{
	  putback_buf[num_putback++] = utf8[3];
	  putback_buf[num_putback++] = utf8[1];
	  putback_buf[num_putback++] = utf8[2];
	  num_chars = num_print = 0;
	}
      else
	{
	  print_buf[num_print ++] = utf8[0];
	  print_buf[num_print ++] = utf8[1];
	  print_buf[num_print ++] = utf8[2];
	  print_buf[num_print ++] = utf8[3];
	  num_chars ++;
	}
    }
  while (1);

  if (num_chars >= string_min)
    {
      /* We know that we have string_min valid characters in print_buf,
	 and there may be more to come in the stream.  Start displaying
	 them.  */

      print_filename_and_address (filename, address + start_point);

      unsigned int i;
      for (i = 0; i < num_print;)
	{
	  if (print_buf[i] < 127)
	    putchar (print_buf[i++]);
	  else
	    i += display_utf8_char (print_buf + i);
	}

      /* OK so now we have to start read unchecked bytes.  */

      /* Find a series of string_min characters.  Put them into print_buf.  */
      do
	{
	  c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
	  if (c == EOF)
	    break;

	  if (! STRING_ISGRAPHIC (c))
	    break;

	  if (c < 127)
	    {
	      putchar (c);
	      continue;
	    }

	  if (c < 0xc0)
	    break;

	  /* We *might* have a UTF-8 sequence.  Time to start peeking.  */
	  unsigned char utf8[4];

	  utf8[0] = c;
	  c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
	  if (c == EOF)
	    break;
	  utf8[1] = c;

	  if ((utf8[1] & 0xc0) != 0x80)
	    {
	      /* Invalid UTF-8.  */
	      putback_buf[num_putback++] = utf8[1];
	      break;
	    }
	  else if ((utf8[0] & 0x20) == 0)
	    {
	      /* Valid 2-byte UTF-8.  */
	      if (unicode_display == unicode_invalid)
		{
		  putback_buf[num_putback++] = utf8[1];
		  break;
		}
	      else
		{
		  (void) display_utf8_char (utf8);
		  continue;
		}
	    }

	  c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
	  if (c == EOF)
	    break;
	  utf8[2] = c;

	  if ((utf8[2] & 0xc0) != 0x80)
	    {
	      /* Invalid UTF-8.  */
	      putback_buf[num_putback++] = utf8[2];
	      putback_buf[num_putback++] = utf8[1];
	      break;
	    }
	  else if ((utf8[0] & 0x10) == 0)
	    {
	      /* Valid 3-byte UTF-8.  */
	      if (unicode_display == unicode_invalid)
		{
		  putback_buf[num_putback++] = utf8[2];
		  putback_buf[num_putback++] = utf8[1];
		  break;
		}
	      else
		{
		  (void) display_utf8_char (utf8);
		  continue;
		}
	    }

	  c = get_unicode_byte (stream, putback_buf, & num_putback, & num_read);
	  if (c == EOF)
	    break;
	  utf8[3] = c;

	  if ((utf8[3] & 0xc0) != 0x80)
	    {
	      /* Invalid UTF-8.  */
	      putback_buf[num_putback++] = utf8[3];
	      putback_buf[num_putback++] = utf8[2];
	      putback_buf[num_putback++] = utf8[1];
	      break;
	    }
	  else if (unicode_display == unicode_invalid)
	    {
	      putback_buf[num_putback++] = utf8[3];
	      putback_buf[num_putback++] = utf8[2];
	      putback_buf[num_putback++] = utf8[1];
	      break;
	    }
	  else
	    /* A valid 4-byte UTF-8 encoding.  */
	    (void) display_utf8_char (utf8);
	}
      while (1);

      if (output_separator)
	fputs (output_separator, stdout);
      else
	putchar ('\n');
    }

  if (c != EOF)
    /* FIXME: Using tail recursion here is lazy, but it works.  */
    print_unicode_stream_body (filename, address + num_read, stream, putback_buf, num_putback, print_buf);
}

/* Display strings read in from STREAM.  Treat any UTF-8 encoded characters
   encountered according to the setting of the unicode_display variable.
   The stream is positioned at ADDRESS and is attached to FILENAME.  */

static void
print_unicode_stream (const char * filename,
		      file_ptr     address,
		      FILE *       stream)
{
  /* Paranoia checks...  */
  if (filename == NULL
      || stream == NULL
      || unicode_display == unicode_default
      || encoding != 'S'
      || encoding_bytes != 1)
    {
      fprintf (stderr, "ICE: bad arguments to print_unicode_stream\n");
      return;
    }

  /* Allocate space for string_min 4-byte utf-8 characters.  */
  size_t amt = string_min;
  amt = (4 * amt) + 1;
  unsigned char * print_buf = xmalloc (amt);
  /* We should never have to put back more than 4 bytes.  */
  unsigned char putback_buf[5];
  unsigned int num_putback = 0;

  print_unicode_stream_body (filename, address, stream, putback_buf, num_putback, print_buf);
  free (print_buf);
}

/* Find the strings in file FILENAME, read from STREAM.
   Assume that STREAM is positioned so that the next byte read
   is at address ADDRESS in the file.

   If STREAM is NULL, do not read from it.
   The caller can supply a buffer of characters
   to be processed before the data in STREAM.
   MAGIC is the address of the buffer and
   MAGICCOUNT is how many characters are in it.
   Those characters come at address ADDRESS and the data in STREAM follow.  */

static void
print_strings (const char *filename, FILE *stream, file_ptr address,
	       int magiccount, char *magic)
{
  if (unicode_display != unicode_default)
    {
      if (magic != NULL)
	print_unicode_buffer (filename, address,
			      (const unsigned char *) magic, magiccount);

      if (stream != NULL)
	print_unicode_stream (filename, address, stream);
      return;
    }

  char *buf = (char *) xmalloc (sizeof (char) * (string_min + 1));

  while (1)
    {
      file_ptr start;
      unsigned int i;
      long c;

      /* See if the next `string_min' chars are all graphic chars.  */
    tryline:
      start = address;
      for (i = 0; i < string_min; i++)
	{
	  c = get_char (stream, &address, &magiccount, &magic);
	  if (c == EOF)
	    {
	      free (buf);
	      return;
	    }

	  if (! STRING_ISGRAPHIC (c))
	    {
	      /* Found a non-graphic.  Try again starting with next byte.  */
	      unget_part_char (c, &address, &magiccount, &magic);
	      goto tryline;
	    }
	  buf[i] = c;
	}

      /* We found a run of `string_min' graphic characters.  Print up
	 to the next non-graphic character.  */
      print_filename_and_address (filename, start);

      buf[i] = '\0';
      fputs (buf, stdout);

      while (1)
	{
	  c = get_char (stream, &address, &magiccount, &magic);
	  if (c == EOF)
	    break;
	  if (! STRING_ISGRAPHIC (c))
	    {
	      unget_part_char (c, &address, &magiccount, &magic);
	      break;
	    }
	  putchar (c);
	}

      if (output_separator)
	fputs (output_separator, stdout);
      else
	putchar ('\n');
    }
  free (buf);
}

static void
usage (FILE *stream, int status)
{
  fprintf (stream, _("Usage: %s [option(s)] [file(s)]\n"), program_name);
  fprintf (stream, _(" Display printable strings in [file(s)] (stdin by default)\n"));
  fprintf (stream, _(" The options are:\n"));

  if (DEFAULT_STRINGS_ALL)
    fprintf (stream, _("\
  -a - --all                Scan the entire file, not just the data section [default]\n\
  -d --data                 Only scan the data sections in the file\n"));
  else
    fprintf (stream, _("\
  -a - --all                Scan the entire file, not just the data section\n\
  -d --data                 Only scan the data sections in the file [default]\n"));

  fprintf (stream, _("\
  -f --print-file-name      Print the name of the file before each string\n\
  -n <number>               Locate & print any sequence of at least <number>\n\
    --bytes=<number>         displayable characters.  (The default is 4).\n\
  -t --radix={o,d,x}        Print the location of the string in base 8, 10 or 16\n\
  -w --include-all-whitespace Include all whitespace as valid string characters\n\
  -o                        An alias for --radix=o\n\
  -T --target=<BFDNAME>     Specify the binary file format\n\
  -e --encoding={s,S,b,l,B,L} Select character size and endianness:\n\
                            s = 7-bit, S = 8-bit, {b,l} = 16-bit, {B,L} = 32-bit\n\
  --unicode={default|show|invalid|hex|escape|highlight}\n\
  -U {d|s|i|x|e|h}          Specify how to treat UTF-8 encoded unicode characters\n\
  -s --output-separator=<string> String used to separate strings in output.\n\
  @<file>                   Read options from <file>\n\
  -h --help                 Display this information\n\
  -v -V --version           Print the program's version number\n"));
  list_supported_targets (program_name, stream);
  if (REPORT_BUGS_TO[0] && status == 0)
    fprintf (stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
  exit (status);
}
