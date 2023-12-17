/* Handling strings that are given partially in the source encoding and
   partially in Unicode.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.

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

/* Specification.  */
#include "xg-mixed-string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "error-progname.h"
#include "flexmember.h"
#include "msgl-ascii.h"
#include "po-charset.h"
#include "unistr.h"
#include "xalloc.h"

#include "xg-pos.h"

#include "gettext.h"
#define _(str) gettext (str)


/* Allocates a single segment.  */
static inline struct mixed_string_segment *
segment_alloc (enum segment_type type, const char *string, size_t length)
{
  struct mixed_string_segment *segment =
    (struct mixed_string_segment *)
    xmalloc (FLEXSIZEOF (struct mixed_string_segment, contents, length));
  segment->type = type;
  segment->length = length;
  memcpy (segment->contents, string, length);
  return segment;
}

/* Clones a single segment.  */
static inline struct mixed_string_segment *
segment_clone (const struct mixed_string_segment *segment)
{
  return segment_alloc (segment->type, segment->contents, segment->length);
}

mixed_string_ty *
mixed_string_alloc_simple (const char *string,
                           lexical_context_ty lcontext,
                           const char *logical_file_name,
                           int line_number)
{
  struct mixed_string *ms = XMALLOC (struct mixed_string);

  if (*string == '\0')
    {
      /* An empty string.  */
      ms->segments = NULL;
      ms->nsegments = 0;
    }
  else
    {
      ms->segments = XNMALLOC (1, struct mixed_string_segment *);
      if ((xgettext_current_source_encoding == po_charset_ascii
           || xgettext_current_source_encoding == po_charset_utf8)
          && is_ascii_string (string))
        /* An optimization.  */
        ms->segments[0] =
          segment_alloc (utf8_encoded, string, strlen (string));
      else
        /* The general case.  */
        ms->segments[0] =
          segment_alloc (source_encoded, string, strlen (string));
      ms->nsegments = 1;
    }
  ms->lcontext = lcontext;
  ms->logical_file_name = logical_file_name;
  ms->line_number = line_number;

  return ms;
}

mixed_string_ty *
mixed_string_alloc_utf8 (const char *string,
                         lexical_context_ty lcontext,
                         const char *logical_file_name,
                         int line_number)
{
  struct mixed_string *ms = XMALLOC (struct mixed_string);

  if (*string == '\0')
    {
      /* An empty string.  */
      ms->segments = NULL;
      ms->nsegments = 0;
    }
  else
    {
      ms->segments = XNMALLOC (1, struct mixed_string_segment *);
      ms->segments[0] = segment_alloc (utf8_encoded, string, strlen (string));
      ms->nsegments = 1;
    }
  ms->lcontext = lcontext;
  ms->logical_file_name = logical_file_name;
  ms->line_number = line_number;

  return ms;
}

mixed_string_ty *
mixed_string_clone (const mixed_string_ty *ms1)
{
  struct mixed_string *ms = XMALLOC (struct mixed_string);
  size_t nsegments = ms1->nsegments;

  if (nsegments == 0)
    {
      ms->segments = NULL;
      ms->nsegments = 0;
    }
  else
    {
      size_t i;

      ms->segments = XNMALLOC (nsegments, struct mixed_string_segment *);
      for (i = 0; i < nsegments; i++)
        ms->segments[i] = segment_clone (ms1->segments[i]);
      ms->nsegments = nsegments;
    }
  ms->lcontext = ms1->lcontext;
  ms->logical_file_name = ms1->logical_file_name;
  ms->line_number = ms1->line_number;

  return ms;
}

char *
mixed_string_contents (const mixed_string_ty *ms)
{
  size_t nsegments = ms->nsegments;
  /* Trivial cases.  */
  if (nsegments == 0)
    return xstrdup ("");
  if (nsegments == 1 && ms->segments[0]->type == utf8_encoded)
    {
      /* Return the segment, with a NUL at the end.  */
      size_t len = ms->segments[0]->length;
      char *string = XNMALLOC (len + 1, char);
      memcpy (string, ms->segments[0]->contents, len);
      string[len] = '\0';
      return string;
    }
  /* General case.  */
  {
    size_t i;

    for (i = 0; i < nsegments - 1; i++)
      if (memchr (ms->segments[i]->contents, '\0', ms->segments[i]->length)
          != NULL)
        {
          /* Segment i contains a NUL character.  Ignore the remaining
             segments.  */
          nsegments = i + 1;
          break;
        }
  }
  {
    char **converted_segments = XNMALLOC (nsegments, char *);
    size_t length;

    length = 0;
    {
      size_t i;

      for (i = 0; i < nsegments; i++)
        if (ms->segments[i]->type == source_encoded)
          {
            char *source_encoded_string;
            char *utf8_encoded_string;

            /* Copy the segment's contents, with a NUL at the end.  */
            {
              size_t len = ms->segments[i]->length;
              source_encoded_string = XNMALLOC (len + 1, char);
              memcpy (source_encoded_string, ms->segments[i]->contents, len);
              source_encoded_string[len] = '\0';
            }
            /* Convert it to UTF-8 encoding.  */
            utf8_encoded_string =
              from_current_source_encoding (source_encoded_string,
                                            ms->lcontext,
                                            ms->logical_file_name,
                                            ms->line_number);
            if (utf8_encoded_string != source_encoded_string)
              free (source_encoded_string);
            converted_segments[i] = utf8_encoded_string;
            length += strlen (utf8_encoded_string);
          }
        else
          length += ms->segments[i]->length;
    }

    {
      char *string = XNMALLOC (length + 1, char);
      {
        char *p;
        size_t i;

        p = string;
        for (i = 0; i < nsegments; i++)
          if (ms->segments[i]->type == source_encoded)
            {
              p = stpcpy (p, converted_segments[i]);
              free (converted_segments[i]);
            }
          else
            {
              memcpy (p, ms->segments[i]->contents, ms->segments[i]->length);
              p += ms->segments[i]->length;
            }
        assert (p == string + length);
        *p = '\0';
      }

      free (converted_segments);
      return string;
    }
  }
}

void
mixed_string_free (mixed_string_ty *ms)
{
  struct mixed_string_segment **segments = ms->segments;
  size_t nsegments = ms->nsegments;
  if (nsegments > 0)
    {
      size_t i;
      for (i = 0; i < nsegments; i++)
        free (segments[i]);
    }
  free (segments);
  free (ms);
}

char *
mixed_string_contents_free1 (mixed_string_ty *ms)
{
  char *contents = mixed_string_contents (ms);
  mixed_string_free (ms);
  return contents;
}

mixed_string_ty *
mixed_string_concat (const mixed_string_ty *ms1,
                     const mixed_string_ty *ms2)
{
  /* Trivial cases.  */
  if (ms2->nsegments == 0)
    return mixed_string_clone (ms1);
  if (ms1->nsegments == 0)
    return mixed_string_clone (ms2);
  /* General case.  */
  {
    struct mixed_string *ms = XMALLOC (struct mixed_string);
    size_t nsegments = ms1->nsegments + ms2->nsegments;
    size_t j;
    if (ms1->segments[ms1->nsegments-1]->type == ms2->segments[0]->type)
      {
        /* Combine the last segment of ms1 with the first segment of ms2.  */
        size_t i;

        nsegments -= 1;
        ms->segments = XNMALLOC (nsegments, struct mixed_string_segment *);
        j = 0;
        for (i = 0; i < ms1->nsegments - 1; i++)
          ms->segments[j++] = segment_clone (ms1->segments[i]);
        {
          size_t len1 = ms1->segments[i]->length;
          size_t len2 = ms2->segments[0]->length;
          struct mixed_string_segment *newseg =
            (struct mixed_string_segment *)
            xmalloc (FLEXSIZEOF (struct mixed_string_segment, contents,
                                 len1 + len2));
          newseg->type = ms2->segments[0]->type;
          newseg->length = len1 + len2;
          memcpy (newseg->contents, ms1->segments[i]->contents, len1);
          memcpy (newseg->contents + len1, ms2->segments[0]->contents, len2);
          ms->segments[j++] = newseg;
        }
        for (i = 1; i < ms2->nsegments; i++)
          ms->segments[j++] = segment_clone (ms2->segments[i]);
      }
    else
      {
        size_t i;

        ms->segments = XNMALLOC (nsegments, struct mixed_string_segment *);
        j = 0;
        for (i = 0; i < ms1->nsegments; i++)
          ms->segments[j++] = segment_clone (ms1->segments[i]);
        for (i = 0; i < ms2->nsegments; i++)
          ms->segments[j++] = segment_clone (ms2->segments[i]);
      }
    assert (j == nsegments);
    ms->nsegments = nsegments;
    ms->lcontext = ms1->lcontext;
    ms->logical_file_name = ms1->logical_file_name;
    ms->line_number = ms1->line_number;

    return ms;
  }
}

mixed_string_ty *
mixed_string_concat_free1 (mixed_string_ty *ms1, const mixed_string_ty *ms2)
{
  /* Trivial cases.  */
  if (ms2->nsegments == 0)
    return ms1;
  if (ms1->nsegments == 0)
    {
      mixed_string_free (ms1);
      return mixed_string_clone (ms2);
    }
  /* General case.  */
  {
    struct mixed_string *ms = XMALLOC (struct mixed_string);
    size_t nsegments = ms1->nsegments + ms2->nsegments;
    size_t j;
    if (ms1->segments[ms1->nsegments-1]->type == ms2->segments[0]->type)
      {
        /* Combine the last segment of ms1 with the first segment of ms2.  */
        size_t i;

        nsegments -= 1;
        ms->segments = XNMALLOC (nsegments, struct mixed_string_segment *);
        j = 0;
        for (i = 0; i < ms1->nsegments - 1; i++)
          ms->segments[j++] = ms1->segments[i];
        {
          size_t len1 = ms1->segments[i]->length;
          size_t len2 = ms2->segments[0]->length;
          struct mixed_string_segment *newseg =
            (struct mixed_string_segment *)
            xmalloc (FLEXSIZEOF (struct mixed_string_segment, contents,
                                 len1 + len2));
          newseg->type = ms2->segments[0]->type;
          newseg->length = len1 + len2;
          memcpy (newseg->contents, ms1->segments[i]->contents, len1);
          memcpy (newseg->contents + len1, ms2->segments[0]->contents, len2);
          ms->segments[j++] = newseg;
        }
        free (ms1->segments[i]);
        for (i = 1; i < ms2->nsegments; i++)
          ms->segments[j++] = segment_clone (ms2->segments[i]);
      }
    else
      {
        size_t i;

        ms->segments = XNMALLOC (nsegments, struct mixed_string_segment *);
        j = 0;
        for (i = 0; i < ms1->nsegments; i++)
          ms->segments[j++] = ms1->segments[i];
        for (i = 0; i < ms2->nsegments; i++)
          ms->segments[j++] = segment_clone (ms2->segments[i]);
      }
    assert (j == nsegments);
    free (ms1->segments);
    ms->nsegments = nsegments;
    ms->lcontext = ms1->lcontext;
    ms->logical_file_name = ms1->logical_file_name;
    ms->line_number = ms1->line_number;
    free (ms1);

    return ms;
  }
}


void
mixed_string_buffer_init (struct mixed_string_buffer *bp,
                          lexical_context_ty lcontext,
                          const char *logical_file_name,
                          int line_number)
{
  bp->segments = NULL;
  bp->nsegments = 0;
  bp->nsegments_allocated = 0;
  bp->curr_type = -1;
  bp->curr_buffer = NULL;
  bp->curr_buflen = 0;
  bp->curr_allocated = 0;
  bp->utf16_surr = 0;
  bp->lcontext = lcontext;
  bp->logical_file_name = logical_file_name;
  bp->line_number = line_number;
}

bool
mixed_string_buffer_is_empty (const struct mixed_string_buffer *bp)
{
  return (bp->nsegments == 0 && bp->curr_buflen == 0);
}

/* Auxiliary function: Ensure count more bytes are available in
   bp->curr_buffer.  */
static inline void
mixed_string_buffer_grow_curr_buffer (struct mixed_string_buffer *bp,
                                      size_t count)
{
  if (bp->curr_buflen + count > bp->curr_allocated)
    {
      size_t new_allocated = 2 * bp->curr_allocated + 10;
      if (new_allocated < bp->curr_buflen + count)
        new_allocated = bp->curr_buflen + count;
      bp->curr_allocated = new_allocated;
      bp->curr_buffer = xrealloc (bp->curr_buffer, new_allocated);
    }
}

/* Auxiliary function: Append a byte to bp->curr.  */
static inline void
mixed_string_buffer_append_to_curr_buffer (struct mixed_string_buffer *bp,
                                           unsigned char c)
{
  if (bp->curr_buflen == bp->curr_allocated)
    {
      bp->curr_allocated = 2 * bp->curr_allocated + 10;
      bp->curr_buffer = xrealloc (bp->curr_buffer, bp->curr_allocated);
    }
  bp->curr_buffer[bp->curr_buflen++] = c;
}

/* Auxiliary function: Assuming bp->curr_type == utf8_encoded, append a
   Unicode character to bp->curr_buffer.  uc must be < 0x110000.  */
static inline void
mixed_string_buffer_append_to_utf8_buffer (struct mixed_string_buffer *bp,
                                           ucs4_t uc)
{
  unsigned char utf8buf[6];
  int count = u8_uctomb (utf8buf, uc, 6);

  if (count < 0)
    /* The caller should have ensured that uc is not out-of-range.  */
    abort ();

  mixed_string_buffer_grow_curr_buffer (bp, count);
  memcpy (bp->curr_buffer + bp->curr_buflen, utf8buf, count);
  bp->curr_buflen += count;
}

/* Auxiliary function: Assuming bp->curr_type == utf8_encoded, handle the
   attempt to append a lone surrogate to bp->curr_buffer.  */
static void
mixed_string_buffer_append_lone_surrogate (struct mixed_string_buffer *bp,
                                           ucs4_t uc)
{
  /* A half surrogate is invalid, therefore use U+FFFD instead.
     It may be valid in a particular programming language.
     But a half surrogate is invalid in UTF-8:
       - RFC 3629 says
           "The definition of UTF-8 prohibits encoding character
            numbers between U+D800 and U+DFFF".
       - Unicode 4.0 chapter 3
         <http://www.unicode.org/versions/Unicode4.0.0/ch03.pdf>
         section 3.9, p.77, says
           "Because surrogate code points are not Unicode scalar
            values, any UTF-8 byte sequence that would otherwise
            map to code points D800..DFFF is ill-formed."
         and in table 3-6, p. 78, does not mention D800..DFFF.
       - The unicode.org FAQ question "How do I convert an unpaired
         UTF-16 surrogate to UTF-8?" has the answer
           "By representing such an unpaired surrogate on its own
            as a 3-byte sequence, the resulting UTF-8 data stream
            would become ill-formed."
     So use U+FFFD instead.  */
  error_with_progname = false;
  error (0, 0, _("%s:%d: warning: lone surrogate U+%04X"),
         logical_file_name, line_number, uc);
  error_with_progname = true;
  mixed_string_buffer_append_to_utf8_buffer (bp, 0xfffd);
}

/* Auxiliary function: Assuming bp->curr_type == utf8_encoded, flush
   bp->utf16_surr into bp->curr_buffer.  */
static inline void
mixed_string_buffer_flush_utf16_surr (struct mixed_string_buffer *bp)
{
  if (bp->utf16_surr != 0)
    {
      mixed_string_buffer_append_lone_surrogate (bp, bp->utf16_surr);
      bp->utf16_surr = 0;
    }
}

/* Auxiliary function: Append a segment to bp->segments.  */
static inline void
mixed_string_buffer_add_segment (struct mixed_string_buffer *bp,
                                 struct mixed_string_segment *newseg)
{
  if (bp->nsegments == bp->nsegments_allocated)
    {
      size_t new_allocated =
        bp->nsegments_allocated = 2 * bp->nsegments_allocated + 1;
      bp->segments =
        (struct mixed_string_segment **)
        xrealloc (bp->segments,
                  new_allocated * sizeof (struct mixed_string_segment *));
    }
  bp->segments[bp->nsegments++] = newseg;
}

/* Auxiliary function: Flush bp->curr_buffer and bp->utf16_surr into
   bp->segments.  */
static void
mixed_string_buffer_flush_curr (struct mixed_string_buffer *bp)
{
  if (bp->curr_type == utf8_encoded)
    mixed_string_buffer_flush_utf16_surr (bp);
  if (bp->curr_type != -1)
    {
      if (bp->curr_buflen > 0)
        {
          struct mixed_string_segment *segment =
            segment_alloc (bp->curr_type, bp->curr_buffer, bp->curr_buflen);
          mixed_string_buffer_add_segment (bp, segment);
        }
      bp->curr_buflen = 0;
    }
}

void
mixed_string_buffer_append_char (struct mixed_string_buffer *bp, int c)
{
  /* Switch to multibyte character mode.  */
  if (bp->curr_type != source_encoded)
    {
      mixed_string_buffer_flush_curr (bp);
      bp->curr_type = source_encoded;
    }

    mixed_string_buffer_append_to_curr_buffer (bp, (unsigned char) c);
}

void
mixed_string_buffer_append_unicode (struct mixed_string_buffer *bp, int c)
{
  /* Switch to Unicode character mode.  */
  if (bp->curr_type != utf8_encoded)
    {
      mixed_string_buffer_flush_curr (bp);
      bp->curr_type = utf8_encoded;
      assert (bp->utf16_surr == 0);
    }

  /* Test whether this character and the previous one form a Unicode
     surrogate character pair.  */
  if (bp->utf16_surr != 0 && (c >= 0xdc00 && c < 0xe000))
    {
      unsigned short utf16buf[2];
      ucs4_t uc;

      utf16buf[0] = bp->utf16_surr;
      utf16buf[1] = c;
      if (u16_mbtouc (&uc, utf16buf, 2) != 2)
        abort ();

      mixed_string_buffer_append_to_utf8_buffer (bp, uc);
      bp->utf16_surr = 0;
    }
  else
    {
      mixed_string_buffer_flush_utf16_surr (bp);

      if (c >= 0xd800 && c < 0xdc00)
        bp->utf16_surr = c;
      else if (c >= 0xdc00 && c < 0xe000)
        mixed_string_buffer_append_lone_surrogate (bp, c);
      else
        mixed_string_buffer_append_to_utf8_buffer (bp, c);
    }
}

void
mixed_string_buffer_destroy (struct mixed_string_buffer *bp)
{
  struct mixed_string_segment **segments = bp->segments;
  size_t nsegments = bp->nsegments;
  if (nsegments > 0)
    {
      size_t i;
      for (i = 0; i < nsegments; i++)
        free (segments[i]);
    }
  free (segments);
  free (bp->curr_buffer);
}

mixed_string_ty *
mixed_string_buffer_result (struct mixed_string_buffer *bp)
{
  mixed_string_buffer_flush_curr (bp);

  {
    struct mixed_string *ms = XMALLOC (struct mixed_string);
    size_t nsegments = bp->nsegments;

    if (nsegments > 0)
      ms->segments =
        (struct mixed_string_segment **)
        xrealloc (bp->segments,
                  nsegments * sizeof (struct mixed_string_segment *));
    else
      {
        assert (bp->segments == NULL);
        ms->segments = NULL;
      }
    ms->nsegments = nsegments;
    ms->lcontext = bp->lcontext;
    ms->logical_file_name = bp->logical_file_name;
    ms->line_number = bp->line_number;

    free (bp->curr_buffer);

    return ms;
  }
}
