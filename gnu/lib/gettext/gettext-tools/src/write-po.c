/* GNU gettext - internationalization aids
   Copyright (C) 1995-1998, 2000-2010, 2012, 2014-2015, 2018-2021, 2023 Free Software Foundation, Inc.

   This file was written by Peter Miller <millerp@canb.auug.org.au>

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
#include "write-po.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_ICONV
# include <iconv.h>
#endif

#include <textstyle.h>

#include "attribute.h"
#include "c-ctype.h"
#include "po-charset.h"
#include "format.h"
#include "unilbrk.h"
#include "msgl-ascii.h"
#include "pos.h"
#include "write-catalog.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "c-strstr.h"
#include "xvasprintf.h"
#include "verify.h"
#include "po-xerror.h"
#include "gettext.h"

/* Our regular abbreviation.  */
#define _(str) gettext (str)


/* =================== Putting together a #, flags line. =================== */


/* Convert IS_FORMAT in the context of programming language LANG to a flag
   string for use in #, flags.  */

const char *
make_format_description_string (enum is_format is_format, const char *lang,
                                bool debug)
{
  static char result[100];

  switch (is_format)
    {
    case possible:
      if (debug)
        {
          sprintf (result, "possible-%s-format", lang);
          break;
        }
      FALLTHROUGH;
    case yes_according_to_context:
    case yes:
      sprintf (result, "%s-format", lang);
      break;
    case no:
      sprintf (result, "no-%s-format", lang);
      break;
    default:
      /* The others have already been filtered out by significant_format_p.  */
      abort ();
    }

  return result;
}


/* Return true if IS_FORMAT is worth mentioning in a #, flags list.  */

bool
significant_format_p (enum is_format is_format)
{
  return is_format != undecided && is_format != impossible;
}


/* Return true if one of IS_FORMAT is worth mentioning in a #, flags list.  */

static bool
has_significant_format_p (const enum is_format is_format[NFORMATS])
{
  size_t i;

  for (i = 0; i < NFORMATS; i++)
    if (significant_format_p (is_format[i]))
      return true;
  return false;
}


/* Convert a RANGE to a freshly allocated string for use in #, flags.  */

char *
make_range_description_string (struct argument_range range)
{
  char *result = xasprintf ("range: %d..%d", range.min, range.max);
  assume (result != NULL);
  return result;
}


/* Convert a wrapping flag DO_WRAP to a string for use in #, flags.  */

static const char *
make_c_width_description_string (enum is_wrap do_wrap)
{
  const char *result = NULL;

  switch (do_wrap)
    {
    case yes:
      result = "wrap";
      break;
    case no:
      result = "no-wrap";
      break;
    default:
      abort ();
    }

  return result;
}


/* ========================== Styling primitives. ========================== */


/* When compiled in src, enable styling support.
   When compiled in libgettextpo, don't enable styling support.  */
#ifdef GETTEXTDATADIR

/* All ostream_t instances are in fact styled_ostream_t instances.  */
#define is_stylable(stream) true

/* Start a run of text belonging to a given CSS class.  */
static inline void
begin_css_class (ostream_t stream, const char *classname)
{
  styled_ostream_begin_use_class ((styled_ostream_t) stream, classname);
}

/* End a run of text belonging to a given CSS class.  */
static inline void
end_css_class (ostream_t stream, const char *classname)
{
  styled_ostream_end_use_class ((styled_ostream_t) stream, classname);
}

#else

#define is_stylable(stream) false
#define begin_css_class(stream,classname) (void)(classname)
#define end_css_class(stream,classname) (void)(classname)

#endif

/* CSS classes at message level.  */
static const char class_header[] = "header";
static const char class_translated[] = "translated";
static const char class_untranslated[] = "untranslated";
static const char class_fuzzy[] = "fuzzy";
static const char class_obsolete[] = "obsolete";

/* CSS classes describing the parts of a message.  */
static const char class_comment[] = "comment";
static const char class_translator_comment[] = "translator-comment";
static const char class_extracted_comment[] = "extracted-comment";
static const char class_reference_comment[] = "reference-comment";
static const char class_reference[] = "reference";
static const char class_flag_comment[] = "flag-comment";
static const char class_flag[] = "flag";
static const char class_fuzzy_flag[] = "fuzzy-flag";
static const char class_previous_comment[] = "previous-comment";
static const char class_previous[] = "previous";
static const char class_msgid[] = "msgid";
static const char class_msgstr[] = "msgstr";
static const char class_keyword[] = "keyword";
static const char class_string[] = "string";

/* CSS classes for the contents of strings.  */
static const char class_text[] = "text";
static const char class_escape_sequence[] = "escape-sequence";
static const char class_format_directive[] = "format-directive";
static const char class_invalid_format_directive[] = "invalid-format-directive";
#if 0
static const char class_added[] = "added";
static const char class_changed[] = "changed";
static const char class_removed[] = "removed";
#endif

/* Per-character attributes.  */
enum
{
  ATTR_ESCAPE_SEQUENCE          = 1 << 0,
  /* The following two are exclusive.  */
  ATTR_FORMAT_DIRECTIVE         = 1 << 1,
  ATTR_INVALID_FORMAT_DIRECTIVE = 1 << 2
};


/* ================ Output parts of a message, as comments. ================ */


/* Output mp->comment as a set of comment lines.  */

static bool print_comment = true;

void
message_print_style_comment (bool flag)
{
  print_comment = flag;
}

void
message_print_comment (const message_ty *mp, ostream_t stream)
{
  if (print_comment && mp->comment != NULL)
    {
      size_t j;

      begin_css_class (stream, class_translator_comment);

      for (j = 0; j < mp->comment->nitems; ++j)
        {
          const char *s = mp->comment->item[j];
          do
            {
              const char *e;
              ostream_write_str (stream, "#");
              if (*s != '\0')
                ostream_write_str (stream, " ");
              e = strchr (s, '\n');
              if (e == NULL)
                {
                  ostream_write_str (stream, s);
                  s = NULL;
                }
              else
                {
                  ostream_write_mem (stream, s, e - s);
                  s = e + 1;
                }
              ostream_write_str (stream, "\n");
            }
          while (s != NULL);
        }

      end_css_class (stream, class_translator_comment);
    }
}


/* Output mp->comment_dot as a set of comment lines.  */

void
message_print_comment_dot (const message_ty *mp, ostream_t stream)
{
  if (mp->comment_dot != NULL)
    {
      size_t j;

      begin_css_class (stream, class_extracted_comment);

      for (j = 0; j < mp->comment_dot->nitems; ++j)
        {
          const char *s = mp->comment_dot->item[j];
          ostream_write_str (stream, "#.");
          if (*s != '\0')
            ostream_write_str (stream, " ");
          ostream_write_str (stream, s);
          ostream_write_str (stream, "\n");
        }

      end_css_class (stream, class_extracted_comment);
    }
}


/* Output mp->filepos as a set of comment lines.  */

static enum filepos_comment_type filepos_comment_type = filepos_comment_full;

void
message_print_comment_filepos (const message_ty *mp, ostream_t stream,
                               const char *charset, bool uniforum,
                               size_t page_width)
{
  if (filepos_comment_type != filepos_comment_none
      && mp->filepos_count != 0)
    {
      size_t filepos_count;
      lex_pos_ty *filepos;

      begin_css_class (stream, class_reference_comment);

      if (filepos_comment_type == filepos_comment_file)
        {
          size_t i;

          filepos_count = 0;
          filepos = XNMALLOC (mp->filepos_count, lex_pos_ty);

          for (i = 0; i < mp->filepos_count; ++i)
            {
              lex_pos_ty *pp = &mp->filepos[i];
              size_t j;

              for (j = 0; j < filepos_count; j++)
                if (strcmp (filepos[j].file_name, pp->file_name) == 0)
                  break;

              if (j == filepos_count)
                {
                  filepos[filepos_count].file_name = pp->file_name;
                  filepos[filepos_count].line_number = (size_t)-1;
                  filepos_count++;
                }
            }
        }
      else
        {
          filepos = mp->filepos;
          filepos_count = mp->filepos_count;
        }

      if (uniforum)
        {
          size_t j;

          for (j = 0; j < filepos_count; ++j)
            {
              lex_pos_ty *pp = &filepos[j];
              const char *cp = pp->file_name;
              char *str;

              while (cp[0] == '.' && cp[1] == '/')
                cp += 2;
              ostream_write_str (stream, "# ");
              begin_css_class (stream, class_reference);
              /* There are two Sun formats to choose from: SunOS and
                 Solaris.  Use the Solaris form here.  */
              str = xasprintf ("File: %s, line: %ld",
                               cp, (long) pp->line_number);
              assume (str != NULL);
              ostream_write_str (stream, str);
              end_css_class (stream, class_reference);
              ostream_write_str (stream, "\n");
              free (str);
            }
        }
      else
        {
          const char *canon_charset;
          size_t column;
          size_t j;

          canon_charset = po_charset_canonicalize (charset);

          ostream_write_str (stream, "#:");
          column = 2;
          for (j = 0; j < filepos_count; ++j)
            {
              lex_pos_ty *pp;
              char buffer[22];
              const char *cp;
              size_t width;

              pp = &filepos[j];
              cp = pp->file_name;
              while (cp[0] == '.' && cp[1] == '/')
                cp += 2;
              if (filepos_comment_type == filepos_comment_file
                  /* Some xgettext input formats, like RST, lack line
                     numbers.  */
                  || pp->line_number == (size_t)(-1))
                buffer[0] = '\0';
              else
                sprintf (buffer, ":%ld", (long) pp->line_number);
              /* File names are usually entirely ASCII.  Therefore strlen is
                 sufficient to determine their printed width.  */
              width = strlen (cp) + strlen (buffer) + 1;
              if (column > 2 && column + width > page_width)
                {
                  ostream_write_str (stream, "\n#:");
                  column = 2;
                }
              ostream_write_str (stream, " ");
              begin_css_class (stream, class_reference);
              if (pos_filename_has_spaces (pp))
                {
                  /* Enclose the file name within U+2068 and U+2069 characters,
                     so that it can be parsed unambiguously.  */
                  if (canon_charset == po_charset_utf8)
                    {
                      ostream_write_str (stream, "\xE2\x81\xA8"); /* U+2068 */
                      ostream_write_str (stream, cp);
                      ostream_write_str (stream, "\xE2\x81\xA9"); /* U+2069 */
                    }
                  else if (canon_charset != NULL
                           && strcmp (canon_charset, "GB18030") == 0)
                    {
                      ostream_write_str (stream, "\x81\x36\xAC\x34"); /* U+2068 */
                      ostream_write_str (stream, cp);
                      ostream_write_str (stream, "\x81\x36\xAC\x35"); /* U+2069 */
                    }
                  else
                    abort ();
                }
              else
                ostream_write_str (stream, cp);
              ostream_write_str (stream, buffer);
              end_css_class (stream, class_reference);
              column += width;
            }
          ostream_write_str (stream, "\n");
        }

      if (filepos != mp->filepos)
        free (filepos);

      end_css_class (stream, class_reference_comment);
    }
}


/* Output mp->is_fuzzy, mp->is_format, mp->range, mp->do_wrap as a comment
   line.  */

void
message_print_comment_flags (const message_ty *mp, ostream_t stream, bool debug)
{
  if ((mp->is_fuzzy && mp->msgstr[0] != '\0')
      || has_significant_format_p (mp->is_format)
      || has_range_p (mp->range)
      || mp->do_wrap == no)
    {
      bool first_flag = true;
      size_t i;

      begin_css_class (stream, class_flag_comment);

      ostream_write_str (stream, "#,");

      /* We don't print the fuzzy flag if the msgstr is empty.  This
         might be introduced by the user but we want to normalize the
         output.  */
      if (mp->is_fuzzy && mp->msgstr[0] != '\0')
        {
          ostream_write_str (stream, " ");
          begin_css_class (stream, class_flag);
          begin_css_class (stream, class_fuzzy_flag);
          ostream_write_str (stream, "fuzzy");
          end_css_class (stream, class_fuzzy_flag);
          end_css_class (stream, class_flag);
          first_flag = false;
        }

      for (i = 0; i < NFORMATS; i++)
        if (significant_format_p (mp->is_format[i]))
          {
            if (!first_flag)
              ostream_write_str (stream, ",");

            ostream_write_str (stream, " ");
            begin_css_class (stream, class_flag);
            ostream_write_str (stream,
                               make_format_description_string (mp->is_format[i],
                                                               format_language[i],
                                                               debug));
            end_css_class (stream, class_flag);
            first_flag = false;
          }

      if (has_range_p (mp->range))
        {
          char *string;

          if (!first_flag)
            ostream_write_str (stream, ",");

          ostream_write_str (stream, " ");
          begin_css_class (stream, class_flag);
          string = make_range_description_string (mp->range);
          ostream_write_str (stream, string);
          free (string);
          end_css_class (stream, class_flag);
          first_flag = false;
        }

      if (mp->do_wrap == no)
        {
          if (!first_flag)
            ostream_write_str (stream, ",");

          ostream_write_str (stream, " ");
          begin_css_class (stream, class_flag);
          ostream_write_str (stream,
                             make_c_width_description_string (mp->do_wrap));
          end_css_class (stream, class_flag);
          first_flag = false;
        }

      ostream_write_str (stream, "\n");

      end_css_class (stream, class_flag_comment);
    }
}


/* ========= Some parameters for use by 'msgdomain_list_print_po'. ========= */


/* This variable controls the extent to which the page width applies.
   True means it applies to message strings and file reference lines.
   False means it applies to file reference lines only.  */
static bool wrap_strings = true;

void
message_page_width_ignore ()
{
  wrap_strings = false;
}


/* These three variables control the output style of the message_print
   function.  Interface functions for them are to be used.  */
static bool indent = false;
static bool uniforum = false;
static bool escape = false;

void
message_print_style_indent ()
{
  indent = true;
}

void
message_print_style_uniforum ()
{
  uniforum = true;
}

void
message_print_style_escape (bool flag)
{
  escape = flag;
}

void
message_print_style_filepos (enum filepos_comment_type type)
{
  filepos_comment_type = type;
}


/* --add-location argument handling.  Return an error indicator.  */
bool
handle_filepos_comment_option (const char *option)
{
  if (option != NULL)
    {
      if (strcmp (option, "never") == 0 || strcmp (option, "no") == 0)
        message_print_style_filepos (filepos_comment_none);
      else if (strcmp (option, "full") == 0 || strcmp (option, "yes") == 0)
        message_print_style_filepos (filepos_comment_full);
      else if (strcmp (option, "file") == 0)
        message_print_style_filepos (filepos_comment_file);
      else
        {
          fprintf (stderr, "invalid --add-location argument: %s\n", option);
          return true;
        }
    }
  else
    /* --add-location is equivalent to --add-location=full.  */
    message_print_style_filepos (filepos_comment_full);
  return false;
}


/* =============== msgdomain_list_print_po() and subroutines. =============== */


/* A version of memcpy optimized for the case n <= 1.  */
static inline void
memcpy_small (void *dst, const void *src, size_t n)
{
  if (n > 0)
    {
      char *q = (char *) dst;
      const char *p = (const char *) src;

      *q = *p;
      if (--n > 0)
        do *++q = *++p; while (--n > 0);
    }
}


/* A version of memset optimized for the case n <= 1.  */
/* Avoid false GCC warning "‘__builtin_memset’ specified bound
   18446744073709551614 exceeds maximum object size 9223372036854775807."
   Cf. <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109995>.  */
#if __GNUC__ >= 7
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
static inline void
memset_small (void *dst, char c, size_t n)
{
  if (n > 0)
    {
      char *p = (char *) dst;

      *p = c;
      if (--n > 0)
        do *++p = c; while (--n > 0);
    }
}


static void
wrap (const message_ty *mp, ostream_t stream,
      const char *line_prefix, int extra_indent, const char *css_class,
      const char *name, const char *value,
      enum is_wrap do_wrap, size_t page_width,
      const char *charset)
{
  const char *canon_charset;
  char *fmtdir;
  char *fmtdirattr;
  const char *s;
  bool first_line;
#if HAVE_ICONV
  const char *envval;
  iconv_t conv;
#endif
  bool weird_cjk;

  canon_charset = po_charset_canonicalize (charset);

#if HAVE_ICONV
  /* The old Solaris/openwin msgfmt and GNU msgfmt <= 0.10.35 don't know
     about multibyte encodings, and require a spurious backslash after
     every multibyte character whose last byte is 0x5C.  Some programs,
     like vim, distribute PO files in this broken format.  It is important
     for such programs that GNU msgmerge continues to support this old
     PO file format when the Makefile requests it.  */
  envval = getenv ("OLD_PO_FILE_OUTPUT");
  if (envval != NULL && *envval != '\0')
    /* Write a PO file in old format, with extraneous backslashes.  */
    conv = (iconv_t)(-1);
  else
    if (canon_charset == NULL)
      /* Invalid PO file encoding.  */
      conv = (iconv_t)(-1);
    else
      /* Avoid glibc-2.1 bug with EUC-KR.  */
# if ((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
     && !defined _LIBICONV_VERSION
      if (strcmp (canon_charset, "EUC-KR") == 0)
        conv = (iconv_t)(-1);
      else
# endif
      /* Avoid Solaris 2.9 bug with GB2312, EUC-TW, BIG5, BIG5-HKSCS, GBK,
         GB18030.  */
# if defined __sun && !defined _LIBICONV_VERSION
      if (   strcmp (canon_charset, "GB2312") == 0
          || strcmp (canon_charset, "EUC-TW") == 0
          || strcmp (canon_charset, "BIG5") == 0
          || strcmp (canon_charset, "BIG5-HKSCS") == 0
          || strcmp (canon_charset, "GBK") == 0
          || strcmp (canon_charset, "GB18030") == 0)
        conv = (iconv_t)(-1);
      else
# endif
      /* Use iconv() to parse multibyte characters.  */
      conv = iconv_open ("UTF-8", canon_charset);

  if (conv != (iconv_t)(-1))
    weird_cjk = false;
  else
#endif
    if (canon_charset == NULL)
      weird_cjk = false;
    else
      weird_cjk = po_is_charset_weird_cjk (canon_charset);

  if (canon_charset == NULL)
    canon_charset = po_charset_ascii;

  /* Determine the extent of format string directives.  */
  fmtdir = NULL;
  fmtdirattr = NULL;
  if (value[0] != '\0')
    {
      bool is_msgstr =
        (strlen (name) >= 6 && memcmp (name, "msgstr", 6) == 0);
        /* or equivalent: = (css_class == class_msgstr) */
      size_t i;

      for (i = 0; i < NFORMATS; i++)
        if (possible_format_p (mp->is_format[i]))
          {
            size_t len = strlen (value);
            struct formatstring_parser *parser = formatstring_parsers[i];
            char *invalid_reason = NULL;
            void *descr;
            const char *fdp;
            const char *fd_end;
            char *fdap;

            fmtdir = XCALLOC (len, char);
            descr = parser->parse (value, is_msgstr, fmtdir, &invalid_reason);
            if (descr != NULL)
              parser->free (descr);

            /* Locate the FMTDIR_* bits and transform the array to an array
               of attributes.  */
            fmtdirattr = XCALLOC (len, char);
            fd_end = fmtdir + len;
            for (fdp = fmtdir, fdap = fmtdirattr; fdp < fd_end; fdp++, fdap++)
              if (*fdp & FMTDIR_START)
                {
                  const char *fdq;
                  for (fdq = fdp; fdq < fd_end; fdq++)
                    if (*fdq & (FMTDIR_END | FMTDIR_ERROR))
                      break;
                  if (!(fdq < fd_end))
                    /* The ->parse method has determined the start of a
                       formatstring directive but not stored a bit indicating
                       its end. It is a bug in the ->parse method.  */
                    abort ();
                  if (*fdq & FMTDIR_ERROR)
                    memset (fdap, ATTR_INVALID_FORMAT_DIRECTIVE, fdq - fdp + 1);
                  else
                    memset (fdap, ATTR_FORMAT_DIRECTIVE, fdq - fdp + 1);
                  fdap += fdq - fdp;
                  fdp = fdq;
                }
              else
                *fdap = 0;

            break;
          }
    }

  /* Loop over the '\n' delimited portions of value.  */
  s = value;
  first_line = true;
  do
    {
      /* The usual escapes, as defined by the ANSI C Standard.  */
#     define is_escape(c) \
        ((c) == '\a' || (c) == '\b' || (c) == '\f' || (c) == '\n' \
         || (c) == '\r' || (c) == '\t' || (c) == '\v')

      const char *es;
      const char *ep;
      size_t portion_len;
      char *portion;
      char *overrides;
      char *attributes;
      char *linebreaks;
      char *pp;
      char *op;
      char *ap;
      int startcol, startcol_after_break, width;
      size_t i;

      for (es = s; *es != '\0'; )
        if (*es++ == '\n')
          break;

      /* Expand escape sequences in each portion.  */
      for (ep = s, portion_len = 0; ep < es; ep++)
        {
          char c = *ep;
          if (is_escape (c))
            portion_len += 2;
          else if (escape && !c_isprint ((unsigned char) c))
            portion_len += 4;
          else if (c == '\\' || c == '"')
            portion_len += 2;
          else
            {
#if HAVE_ICONV
              if (conv != (iconv_t)(-1))
                {
                  /* Skip over a complete multi-byte character.  Don't
                     interpret the second byte of a multi-byte character as
                     ASCII.  This is needed for the BIG5, BIG5-HKSCS, GBK,
                     GB18030, SHIFT_JIS, JOHAB encodings.  */
                  char scratchbuf[64];
                  const char *inptr = ep;
                  size_t insize;
                  char *outptr = &scratchbuf[0];
                  size_t outsize = sizeof (scratchbuf);
                  size_t res;

                  res = (size_t)(-1);
                  for (insize = 1; inptr + insize <= es; insize++)
                    {
                      res = iconv (conv,
                                   (ICONV_CONST char **) &inptr, &insize,
                                   &outptr, &outsize);
                      if (!(res == (size_t)(-1) && errno == EINVAL))
                        break;
                      /* We expect that no input bytes have been consumed
                         so far.  */
                      if (inptr != ep)
                        abort ();
                    }
                  if (res == (size_t)(-1))
                    {
                      if (errno == EILSEQ)
                        {
                          po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0, false,
                                     _("invalid multibyte sequence"));
                          continue;
                        }
                      else if (errno == EINVAL)
                        {
                          /* This could happen if an incomplete
                             multibyte sequence at the end of input
                             bytes.  */
                          po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0, false,
                                     _("incomplete multibyte sequence"));
                          continue;
                        }
                      else
                        abort ();
                    }
                  insize = inptr - ep;
                  portion_len += insize;
                  ep += insize - 1;
                }
              else
#endif
                {
                  if (weird_cjk
                      /* Special handling of encodings with CJK structure.  */
                      && ep + 2 <= es
                      && (unsigned char) ep[0] >= 0x80
                      && (unsigned char) ep[1] >= 0x30)
                    {
                      portion_len += 2;
                      ep += 1;
                    }
                  else
                    portion_len += 1;
                }
            }
        }
      portion = XNMALLOC (portion_len, char);
      overrides = XNMALLOC (portion_len, char);
      attributes = XNMALLOC (portion_len, char);
      for (ep = s, pp = portion, op = overrides, ap = attributes; ep < es; ep++)
        {
          char c = *ep;
          char attr = (fmtdirattr != NULL ? fmtdirattr[ep - value] : 0);
          char brk = UC_BREAK_UNDEFINED;
          /* Don't break inside format directives.  */
          if (attr == ATTR_FORMAT_DIRECTIVE
              && (fmtdir[ep - value] & FMTDIR_START) == 0)
            brk = UC_BREAK_PROHIBITED;
          if (is_escape (c))
            {
              switch (c)
                {
                case '\a': c = 'a'; break;
                case '\b': c = 'b'; break;
                case '\f': c = 'f'; break;
                case '\n': c = 'n'; break;
                case '\r': c = 'r'; break;
                case '\t': c = 't'; break;
                case '\v': c = 'v'; break;
                default: abort ();
                }
              *pp++ = '\\';
              *pp++ = c;
              *op++ = brk;
              *op++ = UC_BREAK_PROHIBITED;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              /* We warn about any use of escape sequences beside
                 '\n' and '\t'.  */
              if (c != 'n' && c != 't')
                {
                  char *error_message =
                    xasprintf (_("internationalized messages should not contain the '\\%c' escape sequence"),
                               c);
                  po_xerror (PO_SEVERITY_WARNING, mp, NULL, 0, 0, false,
                             error_message);
                  free (error_message);
                }
            }
          else if (escape && !c_isprint ((unsigned char) c))
            {
              *pp++ = '\\';
              *pp++ = '0' + (((unsigned char) c >> 6) & 7);
              *pp++ = '0' + (((unsigned char) c >> 3) & 7);
              *pp++ = '0' + ((unsigned char) c & 7);
              *op++ = brk;
              *op++ = UC_BREAK_PROHIBITED;
              *op++ = UC_BREAK_PROHIBITED;
              *op++ = UC_BREAK_PROHIBITED;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
            }
          else if (c == '\\' || c == '"')
            {
              *pp++ = '\\';
              *pp++ = c;
              *op++ = brk;
              *op++ = UC_BREAK_PROHIBITED;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
              *ap++ = attr | ATTR_ESCAPE_SEQUENCE;
            }
          else
            {
#if HAVE_ICONV
              if (conv != (iconv_t)(-1))
                {
                  /* Copy a complete multi-byte character.  Don't
                     interpret the second byte of a multi-byte character as
                     ASCII.  This is needed for the BIG5, BIG5-HKSCS, GBK,
                     GB18030, SHIFT_JIS, JOHAB encodings.  */
                  char scratchbuf[64];
                  const char *inptr = ep;
                  size_t insize;
                  char *outptr = &scratchbuf[0];
                  size_t outsize = sizeof (scratchbuf);
                  size_t res;

                  res = (size_t)(-1);
                  for (insize = 1; inptr + insize <= es; insize++)
                    {
                      res = iconv (conv,
                                   (ICONV_CONST char **) &inptr, &insize,
                                   &outptr, &outsize);
                      if (!(res == (size_t)(-1) && errno == EINVAL))
                        break;
                      /* We expect that no input bytes have been consumed
                         so far.  */
                      if (inptr != ep)
                        abort ();
                    }
                  if (res == (size_t)(-1))
                    {
                      if (errno == EILSEQ)
                        {
                          po_xerror (PO_SEVERITY_ERROR, mp, NULL, 0, 0,
                                     false, _("invalid multibyte sequence"));
                          continue;
                        }
                      else
                        abort ();
                    }
                  insize = inptr - ep;
                  memcpy_small (pp, ep, insize);
                  pp += insize;
                  *op = brk;
                  memset_small (op + 1, UC_BREAK_PROHIBITED, insize - 1);
                  op += insize;
                  memset_small (ap, attr, insize);
                  ap += insize;
                  ep += insize - 1;
                }
              else
#endif
                {
                  if (weird_cjk
                      /* Special handling of encodings with CJK structure.  */
                      && ep + 2 <= es
                      && (unsigned char) c >= 0x80
                      && (unsigned char) ep[1] >= 0x30)
                    {
                      *pp++ = c;
                      ep += 1;
                      *pp++ = *ep;
                      *op++ = brk;
                      *op++ = UC_BREAK_PROHIBITED;
                      *ap++ = attr;
                      *ap++ = attr;
                    }
                  else
                    {
                      *pp++ = c;
                      *op++ = brk;
                      *ap++ = attr;
                    }
                }
            }
        }

      /* Don't break immediately before the "\n" at the end.  */
      if (es > s && es[-1] == '\n')
        overrides[portion_len - 2] = UC_BREAK_PROHIBITED;

      linebreaks = XNMALLOC (portion_len, char);

      /* Subsequent lines after a break are all indented.
         See INDENT-S.  */
      startcol_after_break = (line_prefix ? strlen (line_prefix) : 0);
      if (indent)
        startcol_after_break = (startcol_after_break + extra_indent + 8) & ~7;
      startcol_after_break++;

      /* The line width.  Allow room for the closing quote character.  */
      width = (wrap_strings && do_wrap != no ? page_width : INT_MAX) - 1;
      /* Adjust for indentation of subsequent lines.  */
      width -= startcol_after_break;

    recompute:
      /* The line starts with different things depending on whether it
         is the first line, and if we are using the indented style.
         See INDENT-F.  */
      startcol = (line_prefix ? strlen (line_prefix) : 0);
      if (first_line)
        {
          startcol += strlen (name);
          if (indent)
            startcol = (startcol + extra_indent + 8) & ~7;
          else
            startcol++;
        }
      else
        {
          if (indent)
            startcol = (startcol + extra_indent + 8) & ~7;
        }
      /* Allow room for the opening quote character.  */
      startcol++;
      /* Adjust for indentation of subsequent lines.  */
      startcol -= startcol_after_break;

      /* Do line breaking on the portion.  */
      ulc_width_linebreaks (portion, portion_len, width, startcol, 0,
                            overrides, canon_charset, linebreaks);

      /* If this is the first line, and we are not using the indented
         style, and the line would wrap, then use an empty first line
         and restart.  */
      if (first_line && !indent
          && portion_len > 0
          && (*es != '\0'
              || startcol > width
              || memchr (linebreaks, UC_BREAK_POSSIBLE, portion_len) != NULL))
        {
          if (line_prefix != NULL)
            ostream_write_str (stream, line_prefix);
          begin_css_class (stream, css_class);
          begin_css_class (stream, class_keyword);
          ostream_write_str (stream, name);
          end_css_class (stream, class_keyword);
          ostream_write_str (stream, " ");
          begin_css_class (stream, class_string);
          ostream_write_str (stream, "\"\"");
          end_css_class (stream, class_string);
          end_css_class (stream, css_class);
          ostream_write_str (stream, "\n");
          first_line = false;
          /* Recompute startcol and linebreaks.  */
          goto recompute;
        }

      /* Print the beginning of the line.  This will depend on whether
         this is the first line, and if the indented style is being
         used.  INDENT-F.  */
      {
        int currcol = 0;

        if (line_prefix != NULL)
          {
            ostream_write_str (stream, line_prefix);
            currcol = strlen (line_prefix);
          }
        begin_css_class (stream, css_class);
        if (first_line)
          {
            begin_css_class (stream, class_keyword);
            ostream_write_str (stream, name);
            currcol += strlen (name);
            end_css_class (stream, class_keyword);
            if (indent)
              {
                if (extra_indent > 0)
                  ostream_write_mem (stream, "        ", extra_indent);
                currcol += extra_indent;
                ostream_write_mem (stream, "        ", 8 - (currcol & 7));
                currcol = (currcol + 8) & ~7;
              }
            else
              {
                ostream_write_str (stream, " ");
                currcol++;
              }
            first_line = false;
          }
        else
          {
            if (indent)
              {
                if (extra_indent > 0)
                  ostream_write_mem (stream, "        ", extra_indent);
                currcol += extra_indent;
                ostream_write_mem (stream, "        ", 8 - (currcol & 7));
                currcol = (currcol + 8) & ~7;
              }
          }
      }

      /* Print the portion itself, with linebreaks where necessary.  */
      {
        char currattr = 0;

        begin_css_class (stream, class_string);
        ostream_write_str (stream, "\"");
        begin_css_class (stream, class_text);

        for (i = 0; i < portion_len; i++)
          {
            if (linebreaks[i] == UC_BREAK_POSSIBLE)
              {
                int currcol;

                /* Change currattr so that it becomes 0.  */
                if (currattr & ATTR_ESCAPE_SEQUENCE)
                  {
                    end_css_class (stream, class_escape_sequence);
                    currattr &= ~ATTR_ESCAPE_SEQUENCE;
                  }
                if (currattr & ATTR_FORMAT_DIRECTIVE)
                  {
                    end_css_class (stream, class_format_directive);
                    currattr &= ~ATTR_FORMAT_DIRECTIVE;
                  }
                else if (currattr & ATTR_INVALID_FORMAT_DIRECTIVE)
                  {
                    end_css_class (stream, class_invalid_format_directive);
                    currattr &= ~ATTR_INVALID_FORMAT_DIRECTIVE;
                  }
                if (!(currattr == 0))
                  abort ();

                end_css_class (stream, class_text);
                ostream_write_str (stream, "\"");
                end_css_class (stream, class_string);
                end_css_class (stream, css_class);
                ostream_write_str (stream, "\n");
                currcol = 0;
                /* INDENT-S.  */
                if (line_prefix != NULL)
                  {
                    ostream_write_str (stream, line_prefix);
                    currcol = strlen (line_prefix);
                  }
                begin_css_class (stream, css_class);
                if (indent)
                  {
                    ostream_write_mem (stream, "        ", 8 - (currcol & 7));
                    currcol = (currcol + 8) & ~7;
                  }
                begin_css_class (stream, class_string);
                ostream_write_str (stream, "\"");
                begin_css_class (stream, class_text);
              }
            /* Change currattr so that it matches attributes[i].  */
            if (attributes[i] != currattr)
              {
                /* class_escape_sequence occurs inside class_format_directive
                   and class_invalid_format_directive, so clear it first.  */
                if (currattr & ATTR_ESCAPE_SEQUENCE)
                  {
                    end_css_class (stream, class_escape_sequence);
                    currattr &= ~ATTR_ESCAPE_SEQUENCE;
                  }
                if (~attributes[i] & currattr & ATTR_FORMAT_DIRECTIVE)
                  {
                    end_css_class (stream, class_format_directive);
                    currattr &= ~ATTR_FORMAT_DIRECTIVE;
                  }
                else if (~attributes[i] & currattr & ATTR_INVALID_FORMAT_DIRECTIVE)
                  {
                    end_css_class (stream, class_invalid_format_directive);
                    currattr &= ~ATTR_INVALID_FORMAT_DIRECTIVE;
                  }
                if (attributes[i] & ~currattr & ATTR_FORMAT_DIRECTIVE)
                  {
                    begin_css_class (stream, class_format_directive);
                    currattr |= ATTR_FORMAT_DIRECTIVE;
                  }
                else if (attributes[i] & ~currattr & ATTR_INVALID_FORMAT_DIRECTIVE)
                  {
                    begin_css_class (stream, class_invalid_format_directive);
                    currattr |= ATTR_INVALID_FORMAT_DIRECTIVE;
                  }
                /* class_escape_sequence occurs inside class_format_directive
                   and class_invalid_format_directive, so set it last.  */
                if (attributes[i] & ~currattr & ATTR_ESCAPE_SEQUENCE)
                  {
                    begin_css_class (stream, class_escape_sequence);
                    currattr |= ATTR_ESCAPE_SEQUENCE;
                  }
              }
            ostream_write_mem (stream, &portion[i], 1);
          }

        /* Change currattr so that it becomes 0.  */
        if (currattr & ATTR_ESCAPE_SEQUENCE)
          {
            end_css_class (stream, class_escape_sequence);
            currattr &= ~ATTR_ESCAPE_SEQUENCE;
          }
        if (currattr & ATTR_FORMAT_DIRECTIVE)
          {
            end_css_class (stream, class_format_directive);
            currattr &= ~ATTR_FORMAT_DIRECTIVE;
          }
        else if (currattr & ATTR_INVALID_FORMAT_DIRECTIVE)
          {
            end_css_class (stream, class_invalid_format_directive);
            currattr &= ~ATTR_INVALID_FORMAT_DIRECTIVE;
          }
        if (!(currattr == 0))
          abort ();

        end_css_class (stream, class_text);
        ostream_write_str (stream, "\"");
        end_css_class (stream, class_string);
        end_css_class (stream, css_class);
        ostream_write_str (stream, "\n");
      }

      free (linebreaks);
      free (attributes);
      free (overrides);
      free (portion);

      s = es;
#     undef is_escape
    }
  while (*s);

  if (fmtdirattr != NULL)
    free (fmtdirattr);
  if (fmtdir != NULL)
    free (fmtdir);

#if HAVE_ICONV
  if (conv != (iconv_t)(-1))
    iconv_close (conv);
#endif
}


static void
print_blank_line (ostream_t stream)
{
  if (uniforum)
    {
      begin_css_class (stream, class_comment);
      ostream_write_str (stream, "#\n");
      end_css_class (stream, class_comment);
    }
  else
    ostream_write_str (stream, "\n");
}


static void
message_print (const message_ty *mp, ostream_t stream,
               const char *charset, size_t page_width, bool blank_line,
               bool debug)
{
  int extra_indent;

  /* Separate messages with a blank line.  Uniforum doesn't like blank
     lines, so use an empty comment (unless there already is one).  */
  if (blank_line && (!uniforum
                     || mp->comment == NULL
                     || mp->comment->nitems == 0
                     || mp->comment->item[0][0] != '\0'))
    print_blank_line (stream);

  if (is_header (mp))
    begin_css_class (stream, class_header);
  else if (mp->msgstr[0] == '\0')
    begin_css_class (stream, class_untranslated);
  else if (mp->is_fuzzy)
    begin_css_class (stream, class_fuzzy);
  else
    begin_css_class (stream, class_translated);

  begin_css_class (stream, class_comment);

  /* Print translator comment if available.  */
  message_print_comment (mp, stream);

  /* Print xgettext extracted comments.  */
  message_print_comment_dot (mp, stream);

  /* Print the file position comments.  This will help a human who is
     trying to navigate the sources.  There is no problem of getting
     repeated positions, because duplicates are checked for.  */
  message_print_comment_filepos (mp, stream, charset, uniforum, page_width);

  /* Print flag information in special comment.  */
  message_print_comment_flags (mp, stream, debug);

  /* Print the previous msgid.  This helps the translator when the msgid has
     only slightly changed.  */
  begin_css_class (stream, class_previous_comment);
  if (mp->prev_msgctxt != NULL)
    wrap (mp, stream, "#| ", 0, class_previous, "msgctxt", mp->prev_msgctxt,
          mp->do_wrap, page_width, charset);
  if (mp->prev_msgid != NULL)
    wrap (mp, stream, "#| ", 0, class_previous, "msgid", mp->prev_msgid,
          mp->do_wrap, page_width, charset);
  if (mp->prev_msgid_plural != NULL)
    wrap (mp, stream, "#| ", 0, class_previous, "msgid_plural",
          mp->prev_msgid_plural, mp->do_wrap, page_width, charset);
  end_css_class (stream, class_previous_comment);
  extra_indent = (mp->prev_msgctxt != NULL || mp->prev_msgid != NULL
                  || mp->prev_msgid_plural != NULL
                  ? 3
                  : 0);

  end_css_class (stream, class_comment);

  /* Print each of the message components.  Wrap them nicely so they
     are as readable as possible.  If there is no recorded msgstr for
     this domain, emit an empty string.  */
  if (mp->msgctxt != NULL && !is_ascii_string (mp->msgctxt)
      && po_charset_canonicalize (charset) != po_charset_utf8)
    {
      char *warning_message =
        xasprintf (_("\
The following msgctxt contains non-ASCII characters.\n\
This will cause problems to translators who use a character encoding\n\
different from yours. Consider using a pure ASCII msgctxt instead.\n\
%s\n"), mp->msgctxt);
      po_xerror (PO_SEVERITY_WARNING, mp, NULL, 0, 0, true, warning_message);
      free (warning_message);
    }
  if (!is_ascii_string (mp->msgid)
      && po_charset_canonicalize (charset) != po_charset_utf8)
    {
      char *warning_message =
        xasprintf (_("\
The following msgid contains non-ASCII characters.\n\
This will cause problems to translators who use a character encoding\n\
different from yours. Consider using a pure ASCII msgid instead.\n\
%s\n"), mp->msgid);
      po_xerror (PO_SEVERITY_WARNING, mp, NULL, 0, 0, true, warning_message);
      free (warning_message);
    }
  if (mp->msgctxt != NULL)
    wrap (mp, stream, NULL, extra_indent, class_msgid, "msgctxt", mp->msgctxt,
          mp->do_wrap, page_width, charset);
  wrap (mp, stream, NULL, extra_indent, class_msgid, "msgid", mp->msgid,
        mp->do_wrap, page_width, charset);
  if (mp->msgid_plural != NULL)
    wrap (mp, stream, NULL, extra_indent, class_msgid, "msgid_plural",
          mp->msgid_plural, mp->do_wrap, page_width, charset);

  if (mp->msgid_plural == NULL)
    wrap (mp, stream, NULL, extra_indent, class_msgstr, "msgstr", mp->msgstr,
          mp->do_wrap, page_width, charset);
  else
    {
      char prefix_buf[20];
      unsigned int i;
      const char *p;

      for (p = mp->msgstr, i = 0;
           p < mp->msgstr + mp->msgstr_len;
           p += strlen (p) + 1, i++)
        {
          sprintf (prefix_buf, "msgstr[%u]", i);
          wrap (mp, stream, NULL, extra_indent, class_msgstr, prefix_buf, p,
                mp->do_wrap, page_width, charset);
        }
    }

  if (is_header (mp))
    end_css_class (stream, class_header);
  else if (mp->msgstr[0] == '\0')
    end_css_class (stream, class_untranslated);
  else if (mp->is_fuzzy)
    end_css_class (stream, class_fuzzy);
  else
    end_css_class (stream, class_translated);
}


static void
message_print_obsolete (const message_ty *mp, ostream_t stream,
                        const char *charset, size_t page_width, bool blank_line,
                        bool debug)
{
  int extra_indent;

  /* If msgstr is the empty string we print nothing.  */
  if (mp->msgstr[0] == '\0')
    return;

  /* Separate messages with a blank line.  Uniforum doesn't like blank
     lines, so use an empty comment (unless there already is one).  */
  if (blank_line)
    print_blank_line (stream);

  begin_css_class (stream, class_obsolete);

  begin_css_class (stream, class_comment);

  /* Print translator comment if available.  */
  message_print_comment (mp, stream);

  /* Print xgettext extracted comments (normally empty).  */
  message_print_comment_dot (mp, stream);

  /* Print the file position comments (normally empty).  */
  message_print_comment_filepos (mp, stream, charset, uniforum, page_width);

  /* Print flag information in special comment.
     Preserve only
       - the fuzzy flag, because it is important for the translator when the
         message becomes active again,
       - the no-wrap flag, because we use mp->do_wrap below for the wrapping,
         therefore further processing through 'msgcat' needs to use the same
         value of do_wrap,
       - the *-format flags, because the wrapping depends on these flags (see
         'Don't break inside format directives' comment), therefore further
         processing through 'msgcat' needs to use the same values of is_format.
     This is a trimmed-down variant of message_print_comment_flags.  */
  if (mp->is_fuzzy
      || has_significant_format_p (mp->is_format)
      || mp->do_wrap == no)
    {
      bool first_flag = true;
      size_t i;

      ostream_write_str (stream, "#,");

      if (mp->is_fuzzy)
        {
          ostream_write_str (stream, " fuzzy");
          first_flag = false;
        }

      for (i = 0; i < NFORMATS; i++)
        if (significant_format_p (mp->is_format[i]))
          {
            if (!first_flag)
              ostream_write_str (stream, ",");

            ostream_write_str (stream, " ");
            ostream_write_str (stream,
                               make_format_description_string (mp->is_format[i],
                                                               format_language[i],
                                                               debug));
            first_flag = false;
          }

      if (mp->do_wrap == no)
        {
          if (!first_flag)
            ostream_write_str (stream, ",");

          ostream_write_str (stream, " ");
          ostream_write_str (stream,
                             make_c_width_description_string (mp->do_wrap));
          first_flag = false;
        }

      ostream_write_str (stream, "\n");
    }

  /* Print the previous msgid.  This helps the translator when the msgid has
     only slightly changed.  */
  begin_css_class (stream, class_previous_comment);
  if (mp->prev_msgctxt != NULL)
    wrap (mp, stream, "#~| ", 0, class_previous, "msgctxt", mp->prev_msgctxt,
          mp->do_wrap, page_width, charset);
  if (mp->prev_msgid != NULL)
    wrap (mp, stream, "#~| ", 0, class_previous, "msgid", mp->prev_msgid,
          mp->do_wrap, page_width, charset);
  if (mp->prev_msgid_plural != NULL)
    wrap (mp, stream, "#~| ", 0, class_previous, "msgid_plural",
          mp->prev_msgid_plural, mp->do_wrap, page_width, charset);
  end_css_class (stream, class_previous_comment);
  extra_indent = (mp->prev_msgctxt != NULL || mp->prev_msgid != NULL
                  || mp->prev_msgid_plural != NULL
                  ? 1
                  : 0);

  end_css_class (stream, class_comment);

  /* Print each of the message components.  Wrap them nicely so they
     are as readable as possible.  */
  if (mp->msgctxt != NULL && !is_ascii_string (mp->msgctxt)
      && po_charset_canonicalize (charset) != po_charset_utf8)
    {
      char *warning_message =
        xasprintf (_("\
The following msgctxt contains non-ASCII characters.\n\
This will cause problems to translators who use a character encoding\n\
different from yours. Consider using a pure ASCII msgctxt instead.\n\
%s\n"), mp->msgctxt);
      po_xerror (PO_SEVERITY_WARNING, mp, NULL, 0, 0, true, warning_message);
      free (warning_message);
    }
  if (!is_ascii_string (mp->msgid)
      && po_charset_canonicalize (charset) != po_charset_utf8)
    {
      char *warning_message =
        xasprintf (_("\
The following msgid contains non-ASCII characters.\n\
This will cause problems to translators who use a character encoding\n\
different from yours. Consider using a pure ASCII msgid instead.\n\
%s\n"), mp->msgid);
      po_xerror (PO_SEVERITY_WARNING, mp, NULL, 0, 0, true, warning_message);
      free (warning_message);
    }
  if (mp->msgctxt != NULL)
    wrap (mp, stream, "#~ ", extra_indent, class_msgid, "msgctxt", mp->msgctxt,
          mp->do_wrap, page_width, charset);
  wrap (mp, stream, "#~ ", extra_indent, class_msgid, "msgid", mp->msgid,
        mp->do_wrap, page_width, charset);
  if (mp->msgid_plural != NULL)
    wrap (mp, stream, "#~ ", extra_indent, class_msgid, "msgid_plural",
          mp->msgid_plural, mp->do_wrap, page_width, charset);

  if (mp->msgid_plural == NULL)
    wrap (mp, stream, "#~ ", extra_indent, class_msgstr, "msgstr", mp->msgstr,
          mp->do_wrap, page_width, charset);
  else
    {
      char prefix_buf[20];
      unsigned int i;
      const char *p;

      for (p = mp->msgstr, i = 0;
           p < mp->msgstr + mp->msgstr_len;
           p += strlen (p) + 1, i++)
        {
          sprintf (prefix_buf, "msgstr[%u]", i);
          wrap (mp, stream, "#~ ", extra_indent, class_msgstr, prefix_buf, p,
                mp->do_wrap, page_width, charset);
        }
    }

  end_css_class (stream, class_obsolete);
}


static void
msgdomain_list_print_po (msgdomain_list_ty *mdlp, ostream_t stream,
                         size_t page_width, bool debug)
{
  size_t j, k;
  bool blank_line;

  /* Write out the messages for each domain.  */
  blank_line = false;
  for (k = 0; k < mdlp->nitems; k++)
    {
      message_list_ty *mlp;
      const char *header;
      const char *charset;
      char *allocated_charset;

      /* If the first domain is the default, don't bother emitting
         the domain name, because it is the default.  */
      if (!(k == 0
            && strcmp (mdlp->item[k]->domain, MESSAGE_DOMAIN_DEFAULT) == 0))
        {
          if (blank_line)
            print_blank_line (stream);
          begin_css_class (stream, class_keyword);
          ostream_write_str (stream, "domain");
          end_css_class (stream, class_keyword);
          ostream_write_str (stream, " ");
          begin_css_class (stream, class_string);
          ostream_write_str (stream, "\"");
          begin_css_class (stream, class_text);
          ostream_write_str (stream, mdlp->item[k]->domain);
          end_css_class (stream, class_text);
          ostream_write_str (stream, "\"");
          end_css_class (stream, class_string);
          ostream_write_str (stream, "\n");
          blank_line = true;
        }

      mlp = mdlp->item[k]->messages;

      /* Search the header entry.  */
      header = NULL;
      for (j = 0; j < mlp->nitems; ++j)
        if (is_header (mlp->item[j]) && !mlp->item[j]->obsolete)
          {
            header = mlp->item[j]->msgstr;
            break;
          }

      /* Extract the charset name.  */
      charset = "ASCII";
      allocated_charset = NULL;
      if (header != NULL)
        {
          const char *charsetstr = c_strstr (header, "charset=");

          if (charsetstr != NULL)
            {
              size_t len;

              charsetstr += strlen ("charset=");
              len = strcspn (charsetstr, " \t\n");
              allocated_charset = (char *) xmalloca (len + 1);
              memcpy (allocated_charset, charsetstr, len);
              allocated_charset[len] = '\0';
              charset = allocated_charset;

              /* Treat the dummy default value as if it were absent.  */
              if (strcmp (charset, "CHARSET") == 0)
                charset = "ASCII";
            }
        }

      /* Write out each of the messages for this domain.  */
      for (j = 0; j < mlp->nitems; ++j)
        if (!mlp->item[j]->obsolete)
          {
            message_print (mlp->item[j], stream, charset, page_width,
                           blank_line, debug);
            blank_line = true;
          }

      /* Write out each of the obsolete messages for this domain.  */
      for (j = 0; j < mlp->nitems; ++j)
        if (mlp->item[j]->obsolete)
          {
            message_print_obsolete (mlp->item[j], stream, charset, page_width,
                                    blank_line, debug);
            blank_line = true;
          }

      if (allocated_charset != NULL)
        freea (allocated_charset);
    }
}


/* Describes a PO file in .po syntax.  */
const struct catalog_output_format output_format_po =
{
  msgdomain_list_print_po,              /* print */
  false,                                /* requires_utf8 */
  true,                                 /* requires_utf8_for_filenames_with_spaces */
  true,                                 /* supports_color */
  true,                                 /* supports_multiple_domains */
  true,                                 /* supports_contexts */
  true,                                 /* supports_plurals */
  true,                                 /* sorts_obsoletes_to_end */
  false,                                /* alternative_is_po */
  false                                 /* alternative_is_java_class */
};
