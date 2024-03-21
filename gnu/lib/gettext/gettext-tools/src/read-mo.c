/* Reading binary .mo files.
   Copyright (C) 1995-1998, 2000-2007, 2014-2015, 2017, 2020 Free Software Foundation, Inc.
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

/* Specification.  */
#include "read-mo.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* These two include files describe the binary .mo format.  */
#include "gmo.h"
#include "hash-string.h"

#include "error.h"
#include "xalloc.h"
#include "binary-io.h"
#include "message.h"
#include "format.h"
#include "gettext.h"
#include "xsize.h"

#define _(str) gettext (str)


enum mo_endianness
{
  MO_LITTLE_ENDIAN,
  MO_BIG_ENDIAN
};

/* We read the file completely into memory.  This is more efficient than
   lots of lseek().  This struct represents the .mo file in memory.  */
struct binary_mo_file
{
  const char *filename;
  char *data;
  size_t size;
  enum mo_endianness endian;
};


/* Read the contents of the given input stream.  */
static void
read_binary_mo_file (struct binary_mo_file *bfp,
                     FILE *fp, const char *filename)
{
  char *buf = NULL;
  size_t alloc = 0;
  size_t size = 0;
  size_t count;

  while (!feof (fp))
    {
      const size_t increment = 4096;
      if (size + increment > alloc)
        {
          alloc = alloc + alloc / 2;
          if (alloc < size + increment)
            alloc = size + increment;
          buf = (char *) xrealloc (buf, alloc);
        }
      count = fread (buf + size, 1, increment, fp);
      if (count == 0)
        {
          if (ferror (fp))
            error (EXIT_FAILURE, errno, _("error while reading \"%s\""),
                   filename);
        }
      else
        size += count;
    }
  buf = (char *) xrealloc (buf, size);
  bfp->filename = filename;
  bfp->data = buf;
  bfp->size = size;
}

/* Get a 32-bit number from the file, at the given file position.  */
static nls_uint32
get_uint32 (const struct binary_mo_file *bfp, size_t offset)
{
  nls_uint32 b0, b1, b2, b3;
  size_t end = xsum (offset, 4);

  if (size_overflow_p (end) || end > bfp->size)
    error (EXIT_FAILURE, 0, _("file \"%s\" is truncated"), bfp->filename);

  b0 = *(unsigned char *) (bfp->data + offset + 0);
  b1 = *(unsigned char *) (bfp->data + offset + 1);
  b2 = *(unsigned char *) (bfp->data + offset + 2);
  b3 = *(unsigned char *) (bfp->data + offset + 3);
  if (bfp->endian == MO_LITTLE_ENDIAN)
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
  else
    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

/* Get a static string from the file, at the given file position.  */
static char *
get_string (const struct binary_mo_file *bfp, size_t offset, size_t *lengthp)
{
  /* See 'struct string_desc'.  */
  nls_uint32 s_length = get_uint32 (bfp, offset);
  nls_uint32 s_offset = get_uint32 (bfp, offset + 4);
  size_t s_end = xsum3 (s_offset, s_length, 1);

  if (size_overflow_p (s_end) || s_end > bfp->size)
    error (EXIT_FAILURE, 0, _("file \"%s\" is truncated"), bfp->filename);
  if (bfp->data[s_offset + s_length] != '\0')
    error (EXIT_FAILURE, 0,
           _("file \"%s\" contains a not NUL terminated string"),
           bfp->filename);

  *lengthp = s_length + 1;
  return bfp->data + s_offset;
}

/* Get a system dependent string from the file, at the given file position.  */
static char *
get_sysdep_string (const struct binary_mo_file *bfp, size_t offset,
                   const struct mo_file_header *header, size_t *lengthp)
{
  /* See 'struct sysdep_string'.  */
  size_t length;
  char *string;
  size_t i;
  char *p;
  nls_uint32 s_offset;

  /* Compute the length.  */
  s_offset = get_uint32 (bfp, offset);
  length = 0;
  for (i = 4; ; i += 8)
    {
      nls_uint32 segsize = get_uint32 (bfp, offset + i);
      nls_uint32 sysdepref = get_uint32 (bfp, offset + i + 4);
      nls_uint32 sysdep_segment_offset;
      nls_uint32 ss_length;
      nls_uint32 ss_offset;
      size_t ss_end;
      size_t s_end;
      size_t n;

      s_end = xsum (s_offset, segsize);
      if (size_overflow_p (s_end) || s_end > bfp->size)
        error (EXIT_FAILURE, 0, _("file \"%s\" is truncated"), bfp->filename);
      length += segsize;
      s_offset += segsize;

      if (sysdepref == SEGMENTS_END)
        {
          /* The last static segment must end in a NUL.  */
          if (!(segsize > 0 && bfp->data[s_offset - 1] == '\0'))
            /* Invalid.  */
            error (EXIT_FAILURE, 0,
                   _("file \"%s\" contains a not NUL terminated system dependent string"),
                   bfp->filename);
          break;
        }
      if (sysdepref >= header->n_sysdep_segments)
        /* Invalid.  */
        error (EXIT_FAILURE, 0, _("file \"%s\" is not in GNU .mo format"),
               bfp->filename);
      /* See 'struct sysdep_segment'.  */
      sysdep_segment_offset = header->sysdep_segments_offset + sysdepref * 8;
      ss_length = get_uint32 (bfp, sysdep_segment_offset);
      ss_offset = get_uint32 (bfp, sysdep_segment_offset + 4);
      ss_end = xsum (ss_offset, ss_length);
      if (size_overflow_p (ss_end) || ss_end > bfp->size)
        error (EXIT_FAILURE, 0, _("file \"%s\" is truncated"), bfp->filename);
      if (!(ss_length > 0 && bfp->data[ss_end - 1] == '\0'))
        {
          char location[30];
          sprintf (location, "sysdep_segment[%u]", (unsigned int) sysdepref);
          error (EXIT_FAILURE, 0,
                 _("file \"%s\" contains a not NUL terminated string, at %s"),
                 bfp->filename, location);
        }
      n = strlen (bfp->data + ss_offset);
      length += (n > 1 ? 1 + n + 1 : n);
    }

  /* Allocate and fill the string.  */
  string = XNMALLOC (length, char);
  p = string;
  s_offset = get_uint32 (bfp, offset);
  for (i = 4; ; i += 8)
    {
      nls_uint32 segsize = get_uint32 (bfp, offset + i);
      nls_uint32 sysdepref = get_uint32 (bfp, offset + i + 4);
      nls_uint32 sysdep_segment_offset;
      nls_uint32 ss_length;
      nls_uint32 ss_offset;
      size_t n;

      memcpy (p, bfp->data + s_offset, segsize);
      p += segsize;
      s_offset += segsize;

      if (sysdepref == SEGMENTS_END)
        break;
      if (sysdepref >= header->n_sysdep_segments)
        abort ();
      /* See 'struct sysdep_segment'.  */
      sysdep_segment_offset = header->sysdep_segments_offset + sysdepref * 8;
      ss_length = get_uint32 (bfp, sysdep_segment_offset);
      ss_offset = get_uint32 (bfp, sysdep_segment_offset + 4);
      if (ss_offset + ss_length > bfp->size)
        abort ();
      if (!(ss_length > 0 && bfp->data[ss_offset + ss_length - 1] == '\0'))
        abort ();
      n = strlen (bfp->data + ss_offset);
      if (n > 1)
        *p++ = '<';
      memcpy (p, bfp->data + ss_offset, n);
      p += n;
      if (n > 1)
        *p++ = '>';
    }

  if (p != string + length)
    abort ();

  *lengthp = length;
  return string;
}

/* Reads an existing .mo file and adds the messages to mlp.  */
void
read_mo_file (message_list_ty *mlp, const char *filename)
{
  FILE *fp;
  struct binary_mo_file bf;
  struct mo_file_header header;
  unsigned int i;
  static lex_pos_ty pos = { __FILE__, __LINE__ };

  if (strcmp (filename, "-") == 0 || strcmp (filename, "/dev/stdin") == 0)
    {
      fp = stdin;
      SET_BINARY (fileno (fp));
    }
  else
    {
      fp = fopen (filename, "rb");
      if (fp == NULL)
        error (EXIT_FAILURE, errno,
               _("error while opening \"%s\" for reading"), filename);
    }

  /* Read the file contents into memory.  */
  read_binary_mo_file (&bf, fp, filename);

  /* Get a 32-bit number from the file header.  */
# define GET_HEADER_FIELD(field) \
    get_uint32 (&bf, offsetof (struct mo_file_header, field))

  /* We must grope the file to determine which endian it is.
     Perversity of the universe tends towards maximum, so it will
     probably not match the currently executing architecture.  */
  bf.endian = MO_BIG_ENDIAN;
  header.magic = GET_HEADER_FIELD (magic);
  if (header.magic != _MAGIC)
    {
      bf.endian = MO_LITTLE_ENDIAN;
      header.magic = GET_HEADER_FIELD (magic);
      if (header.magic != _MAGIC)
        {
        unrecognised:
          error (EXIT_FAILURE, 0, _("file \"%s\" is not in GNU .mo format"),
                 filename);
        }
    }

  header.revision = GET_HEADER_FIELD (revision);

  /* We support only the major revisions 0 and 1.  */
  switch (header.revision >> 16)
    {
    case 0:
    case 1:
      /* Fill the header parts that apply to major revisions 0 and 1.  */
      header.nstrings = GET_HEADER_FIELD (nstrings);
      header.orig_tab_offset = GET_HEADER_FIELD (orig_tab_offset);
      header.trans_tab_offset = GET_HEADER_FIELD (trans_tab_offset);
      header.hash_tab_size = GET_HEADER_FIELD (hash_tab_size);
      header.hash_tab_offset = GET_HEADER_FIELD (hash_tab_offset);

      /* The following verifications attempt to ensure that 'msgunfmt' complains
         about a .mo file that may make libintl crash at run time.  */

      /* Verify that the array of messages is sorted.  */
      {
        char *prev_msgid = NULL;

        for (i = 0; i < header.nstrings; i++)
          {
            char *msgid;
            size_t msgid_len;

            msgid = get_string (&bf, header.orig_tab_offset + i * 8,
                                &msgid_len);
            if (i == 0)
              prev_msgid = msgid;
            else
              {
                if (!(strcmp (prev_msgid, msgid) < 0))
                  error (EXIT_FAILURE, 0,
                         _("file \"%s\" is not in GNU .mo format: The array of messages is not sorted."),
                         filename);
              }
          }
      }

      /* Verify the hash table.  */
      if (header.hash_tab_size > 0)
        {
          char *seen;
          unsigned int j;

          /* Verify the hash table's size.  */
          if (!(header.hash_tab_size > 2))
            error (EXIT_FAILURE, 0,
                   _("file \"%s\" is not in GNU .mo format: The hash table size is invalid."),
                   filename);

          /* Verify that the non-empty hash table entries contain the values
             1, ..., nstrings, each exactly once.  */
          seen = (char *) xcalloc (header.nstrings, 1);
          for (j = 0; j < header.hash_tab_size; j++)
            {
              nls_uint32 entry =
                get_uint32 (&bf, header.hash_tab_offset + j * 4);

              if (entry != 0)
                {
                  i = entry - 1;
                  if (!(i < header.nstrings && seen[i] == 0))
                    error (EXIT_FAILURE, 0,
                           _("file \"%s\" is not in GNU .mo format: The hash table contains invalid entries."),
                           filename);
                  seen[i] = 1;
                }
            }
          for (i = 0; i < header.nstrings; i++)
            if (seen[i] == 0)
              error (EXIT_FAILURE, 0, _("file \"%s\" is not in GNU .mo format: Some messages are not present in the hash table."),
                     filename);
          free (seen);

          /* Verify that the hash table lookup algorithm finds the entry for
             each message.  */
          for (i = 0; i < header.nstrings; i++)
            {
              size_t msgid_len;
              char *msgid = get_string (&bf, header.orig_tab_offset + i * 8,
                                        &msgid_len);
              nls_uint32 hash_val = hash_string (msgid);
              nls_uint32 idx = hash_val % header.hash_tab_size;
              nls_uint32 incr = 1 + (hash_val % (header.hash_tab_size - 2));
              for (;;)
                {
                  nls_uint32 entry =
                    get_uint32 (&bf, header.hash_tab_offset + idx * 4);

                  if (entry == 0)
                    error (EXIT_FAILURE, 0,
                           _("file \"%s\" is not in GNU .mo format: Some messages are at a wrong index in the hash table."),
                           filename);
                  if (entry == i + 1)
                    break;

                  if (idx >= header.hash_tab_size - incr)
                    idx -= header.hash_tab_size - incr;
                  else
                    idx += incr;
                }
            }
        }

      for (i = 0; i < header.nstrings; i++)
        {
          message_ty *mp;
          char *msgctxt;
          char *msgid;
          size_t msgid_len;
          char *separator;
          char *msgstr;
          size_t msgstr_len;

          /* Read the msgctxt and msgid.  */
          msgid = get_string (&bf, header.orig_tab_offset + i * 8,
                              &msgid_len);
          /* Split into msgctxt and msgid.  */
          separator = strchr (msgid, MSGCTXT_SEPARATOR);
          if (separator != NULL)
            {
              /* The part before the MSGCTXT_SEPARATOR is the msgctxt.  */
              *separator = '\0';
              msgctxt = msgid;
              msgid = separator + 1;
              msgid_len -= msgid - msgctxt;
            }
          else
            msgctxt = NULL;

          /* Read the msgstr.  */
          msgstr = get_string (&bf, header.trans_tab_offset + i * 8,
                               &msgstr_len);

          mp = message_alloc (msgctxt,
                              msgid,
                              (strlen (msgid) + 1 < msgid_len
                               ? msgid + strlen (msgid) + 1
                               : NULL),
                              msgstr, msgstr_len,
                              &pos);
          message_list_append (mlp, mp);
        }

      switch (header.revision & 0xffff)
        {
        case 0:
          break;
        case 1:
        default:
          /* Fill the header parts that apply to minor revision >= 1.  */
          header.n_sysdep_segments = GET_HEADER_FIELD (n_sysdep_segments);
          header.sysdep_segments_offset =
            GET_HEADER_FIELD (sysdep_segments_offset);
          header.n_sysdep_strings = GET_HEADER_FIELD (n_sysdep_strings);
          header.orig_sysdep_tab_offset =
            GET_HEADER_FIELD (orig_sysdep_tab_offset);
          header.trans_sysdep_tab_offset =
            GET_HEADER_FIELD (trans_sysdep_tab_offset);

          for (i = 0; i < header.n_sysdep_strings; i++)
            {
              message_ty *mp;
              char *msgctxt;
              char *msgid;
              size_t msgid_len;
              char *separator;
              char *msgstr;
              size_t msgstr_len;
              nls_uint32 offset;
              size_t f;

              /* Read the msgctxt and msgid.  */
              offset = get_uint32 (&bf, header.orig_sysdep_tab_offset + i * 4);
              msgid = get_sysdep_string (&bf, offset, &header, &msgid_len);
              /* Split into msgctxt and msgid.  */
              separator = strchr (msgid, MSGCTXT_SEPARATOR);
              if (separator != NULL)
                {
                  /* The part before the MSGCTXT_SEPARATOR is the msgctxt.  */
                  *separator = '\0';
                  msgctxt = msgid;
                  msgid = separator + 1;
                  msgid_len -= msgid - msgctxt;
                }
              else
                msgctxt = NULL;

              /* Read the msgstr.  */
              offset = get_uint32 (&bf, header.trans_sysdep_tab_offset + i * 4);
              msgstr = get_sysdep_string (&bf, offset, &header, &msgstr_len);

              mp = message_alloc (msgctxt,
                                  msgid,
                                  (strlen (msgid) + 1 < msgid_len
                                   ? msgid + strlen (msgid) + 1
                                   : NULL),
                                  msgstr, msgstr_len,
                                  &pos);

              /* Only messages with c-format or objc-format annotation are
                 recognized as having system-dependent strings by msgfmt.
                 Which one of the two, we don't know.  We have to guess,
                 assuming that c-format is more probable than objc-format and
                 that the .mo was likely produced by "msgfmt -c".  */
              for (f = format_c; ; f = format_objc)
                {
                  bool valid = true;
                  struct formatstring_parser *parser = formatstring_parsers[f];
                  const char *str_end;
                  const char *str;

                  str_end = msgid + msgid_len;
                  for (str = msgid; str < str_end; str += strlen (str) + 1)
                    {
                      char *invalid_reason = NULL;
                      void *descr =
                        parser->parse (str, false, NULL, &invalid_reason);

                      if (descr != NULL)
                        parser->free (descr);
                      else
                        {
                          free (invalid_reason);
                          valid = false;
                          break;
                        }
                    }
                  if (valid)
                    {
                      str_end = msgstr + msgstr_len;
                      for (str = msgstr; str < str_end; str += strlen (str) + 1)
                        {
                          char *invalid_reason = NULL;
                          void *descr =
                            parser->parse (str, true, NULL, &invalid_reason);

                          if (descr != NULL)
                            parser->free (descr);
                          else
                            {
                              free (invalid_reason);
                              valid = false;
                              break;
                            }
                        }
                    }

                  if (valid)
                    {
                      /* Found the most likely among c-format, objc-format.  */
                      mp->is_format[f] = yes;
                      break;
                    }

                  /* Try next f.  */
                  if (f == format_objc)
                    break;
                }

              message_list_append (mlp, mp);
            }
          break;
        }
      break;

    default:
      goto unrecognised;
    }

  if (fp != stdin)
    fclose (fp);
}
