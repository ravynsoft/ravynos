/* markup.c -- simple XML-like parser
   Copyright (C) 2015, 2018, 2020 Free Software Foundation, Inc.

   This file is not part of the GNU gettext program, but is used with
   GNU gettext.

   This is a stripped down version of GLib's gmarkup.c.  The original
   copyright notice is as follows:
*/

/* gmarkup.c - Simple XML-like parser
 *
 *  Copyright 2000, 2003 Red Hat, Inc.
 *  Copyright 2007, 2008 Ryan Lortie <desrt@desrt.ca>
 *
 * GLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with GLib; see the file COPYING.LIB.  If not,
 * see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Specification */
#include "markup.h"

#include "c-ctype.h"
#include "gettext.h"
#include "gl_linked_list.h"
#include "gl_xlist.h"
#include "unictype.h"
#include "unistr.h"
#include "xalloc.h"
#include "xvasprintf.h"

#define _(s) gettext(s)

/**
 * The "markup" parser is intended to parse a simple markup format
 * that's a subset of XML.  This is a small, efficient, easy-to-use
 * parser.  It should not be used if you expect to interoperate with
 * other applications generating full-scale XML.  However, it's very
 * useful for application data files, config files, etc. where you
 * know your application will be the only one writing the file.
 * Full-scale XML parsers should be able to parse the subset used by
 * markup, so you can easily migrate to full-scale XML at a later
 * time if the need arises.
 *
 * The parser is not guaranteed to signal an error on all invalid XML;
 * the parser may accept documents that an XML parser would not.
 * However, XML documents which are not well-formed (which is a weaker
 * condition than being valid.  See the XML specification
 * <https://www.w3.org/TR/REC-xml/> for definitions of these terms.)
 * are not considered valid GMarkup documents.
 *
 * Simplifications to XML include:
 *
 * - Only UTF-8 encoding is allowed
 *
 * - No user-defined entities
 *
 * - Processing instructions, comments and the doctype declaration
 *   are "passed through" but are not interpreted in any way
 *
 * - No DTD or validation
 *
 * The markup format does support:
 *
 * - Elements
 *
 * - Attributes
 *
 * - 5 standard entities: &amp; &lt; &gt; &quot; &apos;
 *
 * - Character references
 *
 * - Sections marked as CDATA
 */

typedef enum
{
  STATE_START,
  STATE_AFTER_OPEN_ANGLE,
  STATE_AFTER_CLOSE_ANGLE,
  STATE_AFTER_ELISION_SLASH, /* the slash that obviates need for end element */
  STATE_INSIDE_OPEN_TAG_NAME,
  STATE_INSIDE_ATTRIBUTE_NAME,
  STATE_AFTER_ATTRIBUTE_NAME,
  STATE_BETWEEN_ATTRIBUTES,
  STATE_AFTER_ATTRIBUTE_EQUALS_SIGN,
  STATE_INSIDE_ATTRIBUTE_VALUE_SQ,
  STATE_INSIDE_ATTRIBUTE_VALUE_DQ,
  STATE_INSIDE_TEXT,
  STATE_AFTER_CLOSE_TAG_SLASH,
  STATE_INSIDE_CLOSE_TAG_NAME,
  STATE_AFTER_CLOSE_TAG_NAME,
  STATE_INSIDE_PASSTHROUGH,
  STATE_ERROR
} markup_parse_state_ty;

typedef struct
{
  const char *prev_element;
  const markup_parser_ty *prev_parser;
  void *prev_user_data;
} markup_recursion_tracker_ty;

typedef struct
{
  char *buffer;
  size_t bufmax;
  size_t buflen;
} markup_string_ty;

struct _markup_parse_context_ty
{
  const markup_parser_ty *parser;

  markup_parse_flags_ty flags;

  int line_number;
  int char_number;

  markup_parse_state_ty state;

  void *user_data;

  /* A piece of character data or an element that
   * hasn't "ended" yet so we haven't yet called
   * the callback for it.
   */
  markup_string_ty *partial_chunk;

  gl_list_t tag_stack;          /* <markup_string_ty> */

  char **attr_names;
  char **attr_values;
  int cur_attr;
  int alloc_attrs;

  const char *current_text;
  ssize_t current_text_len;
  const char *current_text_end;

  /* used to save the start of the last interesting thingy */
  const char *start;

  const char *iter;

  char *error_text;

  unsigned int document_empty : 1;
  unsigned int parsing : 1;
  unsigned int awaiting_pop : 1;
  int balance;

  /* subparser support */
  gl_list_t subparser_stack;    /* <markup_recursion_tracker_ty *> */
  const char *subparser_element;
};

static markup_string_ty *
markup_string_new (void)
{
  return XZALLOC (markup_string_ty);
}

static char *
markup_string_free (markup_string_ty *string, bool free_segment)
{
  if (free_segment)
    {
      free (string->buffer);
      free (string);
      return NULL;
    }
  else
    {
      char *result = string->buffer;
      free (string);
      return result;
    }
}

static void
markup_string_free1 (markup_string_ty *string)
{
  markup_string_free (string, true);
}

static void
markup_string_truncate (markup_string_ty *string, size_t length)
{
  assert (string && length < string->buflen - 1);
  string->buffer[length] = '\0';
  string->buflen = length;
}

static void
markup_string_append (markup_string_ty *string, const char *to_append,
                      size_t length)
{
  if (string->buflen + length + 1 > string->bufmax)
    {
      string->bufmax *= 2;
      if (string->buflen + length + 1 > string->bufmax)
        string->bufmax = string->buflen + length + 1;
      string->buffer = xrealloc (string->buffer, string->bufmax);
    }
  memcpy (string->buffer + string->buflen, to_append, length);
  string->buffer[length] = '\0';
  string->buflen = length;
}

static inline void
string_blank (markup_string_ty *string)
{
  if (string->bufmax > 0)
    {
      *string->buffer = '\0';
      string->buflen = 0;
    }
}

/* Creates a new parse context.  A parse context is used to parse
   marked-up documents.  You can feed any number of documents into a
   context, as long as no errors occur; once an error occurs, the
   parse context can't continue to parse text (you have to free it and
   create a new parse context).  */
markup_parse_context_ty *
markup_parse_context_new (const markup_parser_ty *parser,
                          markup_parse_flags_ty flags,
                          void *user_data)
{
  markup_parse_context_ty *context;

  assert (parser != NULL);

  context = XMALLOC (markup_parse_context_ty);

  context->parser = parser;
  context->flags = flags;
  context->user_data = user_data;

  context->line_number = 1;
  context->char_number = 1;

  context->partial_chunk = NULL;

  context->state = STATE_START;
  context->tag_stack =
    gl_list_create_empty (GL_LINKED_LIST,
                          NULL, NULL,
                          (gl_listelement_dispose_fn) markup_string_free1,
                          true);
  context->attr_names = NULL;
  context->attr_values = NULL;
  context->cur_attr = -1;
  context->alloc_attrs = 0;

  context->current_text = NULL;
  context->current_text_len = -1;
  context->current_text_end = NULL;

  context->start = NULL;
  context->iter = NULL;

  context->error_text = NULL;

  context->document_empty = true;
  context->parsing = false;

  context->awaiting_pop = false;
  context->subparser_stack =
    gl_list_create_empty (GL_LINKED_LIST,
                          NULL, NULL,
                          (gl_listelement_dispose_fn) free,
                          true);
  context->subparser_element = NULL;

  context->balance = 0;

  return context;
}

static void clear_attributes (markup_parse_context_ty *context);

/* Frees a parse context.  This function can't be called from inside
   one of the markup_parser_ty functions or while a subparser is
   pushed.  */
void
markup_parse_context_free (markup_parse_context_ty *context)
{
  assert (context != NULL);
  assert (!context->parsing);
  assert (gl_list_size (context->subparser_stack) == 0);
  assert (!context->awaiting_pop);

  clear_attributes (context);
  free (context->attr_names);
  free (context->attr_values);

  gl_list_free (context->tag_stack);
  gl_list_free (context->subparser_stack);

  if (context->partial_chunk)
    markup_string_free (context->partial_chunk, true);

  free (context->error_text);

  free (context);
}

static void pop_subparser_stack (markup_parse_context_ty *context);

static void
emit_error (markup_parse_context_ty *context, const char *error_text)
{
  context->state = STATE_ERROR;

  if (context->parser->error)
    (*context->parser->error) (context, error_text, context->user_data);

  /* report the error all the way up to free all the user-data */
  while (gl_list_size (context->subparser_stack) > 0)
    {
      pop_subparser_stack (context);
      context->awaiting_pop = false; /* already been freed */

      if (context->parser->error)
        (*context->parser->error) (context, error_text, context->user_data);
    }

  if (context->error_text)
    free (context->error_text);
  context->error_text = xstrdup (error_text);
}

#define IS_COMMON_NAME_END_CHAR(c) \
  ((c) == '=' || (c) == '/' || (c) == '>' || (c) == ' ')

static bool
slow_name_validate (markup_parse_context_ty *context, const char *name)
{
  const char *p = name;
  ucs4_t uc;

  if (u8_check ((const uint8_t *) name, strlen (name)) != NULL)
    {
      emit_error (context, _("invalid UTF-8 sequence"));
      return false;
    }

  if (!(c_isalpha (*p)
        || (!IS_COMMON_NAME_END_CHAR (*p)
            && (*p == '_'
                || *p == ':'
                || (u8_mbtouc (&uc, (const uint8_t *) name, strlen (name)) > 0
                    && uc_is_alpha (uc))))))
    {
      char *error_text = xasprintf (_("'%s' is not a valid name: %c"),
                                    name, *p);
      emit_error (context, error_text);
      free (error_text);
      return false;
    }

  for (p = (const char *) u8_next (&uc, (const uint8_t *) name);
       p != NULL;
       p = (const char *) u8_next (&uc, (const uint8_t *) p))
    {
      /* is_name_char */
      if (!(c_isalnum (*p) ||
            (!IS_COMMON_NAME_END_CHAR (*p) &&
             (*p == '.' ||
              *p == '-' ||
              *p == '_' ||
              *p == ':' ||
              uc_is_alpha (uc)))))
        {
          char *error_text = xasprintf (_("'%s' is not a valid name: '%c'"),
                                        name, *p);
          emit_error (context, error_text);
          free (error_text);
          return false;
        }
    }
  return true;
}

/*
 * Use me for elements, attributes etc.
 */
static bool
name_validate (markup_parse_context_ty *context, const char *name)
{
  char mask;
  const char *p;

  /* name start char */
  p = name;
  if (IS_COMMON_NAME_END_CHAR (*p)
      || !(c_isalpha (*p) || *p == '_' || *p == ':'))
    goto slow_validate;

  for (mask = *p++; *p != '\0'; p++)
    {
      mask |= *p;

      /* is_name_char */
      if (!(c_isalnum (*p)
            || (!IS_COMMON_NAME_END_CHAR (*p)
                && (*p == '.' || *p == '-' || *p == '_' || *p == ':'))))
        goto slow_validate;
    }

  if (mask & 0x80) /* un-common / non-ascii */
    goto slow_validate;

  return true;

 slow_validate:
  return slow_name_validate (context, name);
}

static bool
text_validate (markup_parse_context_ty *context,
               const char *p,
               int len)
{
  if (u8_check ((const uint8_t *) p, len) != NULL)
    {
      emit_error (context, _("invalid UTF-8 sequence"));
      return false;
    }
  else
    return true;
}

/*
 * re-write the GString in-place, unescaping anything that escaped.
 * most XML does not contain entities, or escaping.
 */
static bool
unescape_string_inplace (markup_parse_context_ty *context,
                         markup_string_ty *string,
                         bool *is_ascii)
{
  char mask, *to;
  const char *from;
  bool normalize_attribute;

  if (string->buflen == 0)
    return true;

  *is_ascii = false;

  /* are we unescaping an attribute or not ? */
  if (context->state == STATE_INSIDE_ATTRIBUTE_VALUE_SQ
      || context->state == STATE_INSIDE_ATTRIBUTE_VALUE_DQ)
    normalize_attribute = true;
  else
    normalize_attribute = false;

  /*
   * Meeks' theorem: unescaping can only shrink text.
   * for &lt; etc. this is obvious, for &#xffff; more
   * thought is required, but this is patently so.
   */
  mask = 0;
  for (from = to = string->buffer; *from != '\0'; from++, to++)
    {
      *to = *from;

      mask |= *to;
      if (normalize_attribute && (*to == '\t' || *to == '\n'))
        *to = ' ';
      if (*to == '\r')
        {
          *to = normalize_attribute ? ' ' : '\n';
          if (from[1] == '\n')
            from++;
        }
      if (*from == '&')
        {
          from++;
          if (*from == '#')
            {
              int base = 10;
              unsigned long l;
              char *end = NULL;

              from++;

              if (*from == 'x')
                {
                  base = 16;
                  from++;
                }

              errno = 0;
              l = strtoul (from, &end, base);

              if (end == from || errno != 0)
                {
                  char *error_text =
                    xasprintf (_("invalid character reference: %s"),
                               errno != 0
                               ? strerror (errno)
                               : _("not a valid number specification"));
                  emit_error (context, error_text);
                  free (error_text);
                  return false;
                }
              else if (*end != ';')
                {
                  char *error_text =
                    xasprintf (_("invalid character reference: %s"),
                               _("no ending ';'"));
                  emit_error (context, error_text);
                  free (error_text);
                  return false;
                }
              else
                {
                  /* characters XML 1.1 permits */
                  if ((0 < l && l <= 0xD7FF) ||
                      (0xE000 <= l && l <= 0xFFFD) ||
                      (0x10000 <= l && l <= 0x10FFFF))
                    {
                      char buf[8];
                      int length;
                      length = u8_uctomb ((uint8_t *) buf, l, 8);
                      memcpy (to, buf, length);
                      to += length - 1;
                      from = end;
                      if (l >= 0x80) /* not ascii */
                        mask |= 0x80;
                    }
                  else
                    {
                      char *error_text =
                        xasprintf (_("invalid character reference: %s"),
                                   _("non-permitted character"));
                      emit_error (context, error_text);
                      free (error_text);
                      return false;
                    }
                }
            }

          else if (strncmp (from, "lt;", 3) == 0)
            {
              *to = '<';
              from += 2;
            }
          else if (strncmp (from, "gt;", 3) == 0)
            {
              *to = '>';
              from += 2;
            }
          else if (strncmp (from, "amp;", 4) == 0)
            {
              *to = '&';
              from += 3;
            }
          else if (strncmp (from, "quot;", 5) == 0)
            {
              *to = '"';
              from += 4;
            }
          else if (strncmp (from, "apos;", 5) == 0)
            {
              *to = '\'';
              from += 4;
            }
          else
            {
              const char *reason;
              char *error_text;

              if (*from == ';')
                reason = _("empty");
              else
                {
                  const char *end = strchr (from, ';');
                  if (end)
                    reason = _("unknown");
                  else
                    reason = _("no ending ';'");
                }
              error_text = xasprintf (_("invalid entity reference: %s"),
                                      reason);
              emit_error (context, error_text);
              free (error_text);
              return false;
            }
        }
    }

  assert (to - string->buffer <= string->buflen);
  if (to - string->buffer != string->buflen)
    markup_string_truncate (string, to - string->buffer);

  *is_ascii = !(mask & 0x80);

  return true;
}

static inline bool
advance_char (markup_parse_context_ty *context)
{
  context->iter++;
  context->char_number++;

  if (context->iter == context->current_text_end)
      return false;

  else if (*context->iter == '\n')
    {
      context->line_number++;
      context->char_number = 1;
    }

  return true;
}

static inline bool
xml_isspace (char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void
skip_spaces (markup_parse_context_ty *context)
{
  do
    {
      if (!xml_isspace (*context->iter))
        return;
    }
  while (advance_char (context));
}

static void
advance_to_name_end (markup_parse_context_ty *context)
{
  do
    {
      if (IS_COMMON_NAME_END_CHAR (*(context->iter)))
        return;
      if (xml_isspace (*(context->iter)))
        return;
    }
  while (advance_char (context));
}

static void
add_to_partial (markup_parse_context_ty *context,
                const char         *text_start,
                const char         *text_end)
{
  if (context->partial_chunk == NULL)
    { /* allocate a new chunk to parse into */

      context->partial_chunk = markup_string_new ();
    }

  if (text_start != text_end)
    markup_string_append (context->partial_chunk,
                          text_start, text_end - text_start);
}

static inline void
truncate_partial (markup_parse_context_ty *context)
{
  if (context->partial_chunk != NULL)
    string_blank (context->partial_chunk);
}

static inline const char*
current_element (markup_parse_context_ty *context)
{
  const markup_string_ty *string = gl_list_get_at (context->tag_stack, 0);
  return string->buffer;
}

static void
pop_subparser_stack (markup_parse_context_ty *context)
{
  markup_recursion_tracker_ty *tracker;

  assert (gl_list_size (context->subparser_stack) > 0);

  tracker = (markup_recursion_tracker_ty *) gl_list_get_at (context->subparser_stack, 0);

  context->awaiting_pop = true;

  context->user_data = tracker->prev_user_data;
  context->parser = tracker->prev_parser;
  context->subparser_element = tracker->prev_element;
  free (tracker);

  gl_list_remove_at (context->subparser_stack, 0);
}

static void
push_partial_as_tag (markup_parse_context_ty *context)
{
  gl_list_add_first (context->tag_stack, context->partial_chunk);
  context->partial_chunk = NULL;
}

static void
pop_tag (markup_parse_context_ty *context)
{
  gl_list_remove_at (context->tag_stack, 0);
}

static void
possibly_finish_subparser (markup_parse_context_ty *context)
{
  if (current_element (context) == context->subparser_element)
    pop_subparser_stack (context);
}

static void
ensure_no_outstanding_subparser (markup_parse_context_ty *context)
{
  context->awaiting_pop = false;
}

static void
add_attribute (markup_parse_context_ty *context, markup_string_ty *string)
{
  if (context->cur_attr + 2 >= context->alloc_attrs)
    {
      context->alloc_attrs += 5; /* silly magic number */
      context->attr_names = xrealloc (context->attr_names, sizeof (char *) * context->alloc_attrs);
      context->attr_values = xrealloc (context->attr_values, sizeof(char *) * context->alloc_attrs);
    }
  context->cur_attr++;
  context->attr_names[context->cur_attr] = xstrdup (string->buffer);
  context->attr_values[context->cur_attr] = NULL;
  context->attr_names[context->cur_attr+1] = NULL;
  context->attr_values[context->cur_attr+1] = NULL;
}

static void
clear_attributes (markup_parse_context_ty *context)
{
  /* Go ahead and free the attributes. */
  for (; context->cur_attr >= 0; context->cur_attr--)
    {
      int pos = context->cur_attr;
      free (context->attr_names[pos]);
      free (context->attr_values[pos]);
      context->attr_names[pos] = context->attr_values[pos] = NULL;
    }
  assert (context->cur_attr == -1);
  assert (context->attr_names == NULL ||
          context->attr_names[0] == NULL);
  assert (context->attr_values == NULL ||
          context->attr_values[0] == NULL);
}

static void
markup_parse_context_push (markup_parse_context_ty *context,
                           const markup_parser_ty *parser,
                           void *user_data)
{
  markup_recursion_tracker_ty *tracker;

  tracker = XMALLOC (markup_recursion_tracker_ty);
  tracker->prev_element = context->subparser_element;
  tracker->prev_parser = context->parser;
  tracker->prev_user_data = context->user_data;

  context->subparser_element = current_element (context);
  context->parser = parser;
  context->user_data = user_data;

  gl_list_add_first (context->subparser_stack, tracker);
}

static void
markup_parse_context_pop (markup_parse_context_ty *context)
{
  if (!context->awaiting_pop)
    possibly_finish_subparser (context);

  assert (context->awaiting_pop);

  context->awaiting_pop = false;
}

/* This has to be a separate function to ensure the alloca's
 * are unwound on exit - otherwise we grow & blow the stack
 * with large documents
 */
static inline void
emit_start_element (markup_parse_context_ty *context)
{
  int i, j = 0;
  const char *start_name;
  const char **attr_names;
  const char **attr_values;

  /* In case we want to ignore qualified tags and we see that we have
   * one here, we push a subparser.  This will ignore all tags inside of
   * the qualified tag.
   *
   * We deal with the end of the subparser from emit_end_element.
   */
  if ((context->flags & MARKUP_IGNORE_QUALIFIED)
      && strchr (current_element (context), ':'))
    {
      static const markup_parser_ty ignore_parser;
      markup_parse_context_push (context, &ignore_parser, NULL);
      clear_attributes (context);
      return;
    }

  attr_names = XCALLOC (context->cur_attr + 2, const char *);
  attr_values = XCALLOC (context->cur_attr + 2, const char *);
  for (i = 0; i < context->cur_attr + 1; i++)
    {
      /* Possibly omit qualified attribute names from the list */
      if ((context->flags & MARKUP_IGNORE_QUALIFIED)
          && strchr (context->attr_names[i], ':'))
        continue;

      attr_names[j] = context->attr_names[i];
      attr_values[j] = context->attr_values[i];
      j++;
    }
  attr_names[j] = NULL;
  attr_values[j] = NULL;

  /* Call user callback for element start */
  start_name = current_element (context);

  if (context->parser->start_element && name_validate (context, start_name))
    (* context->parser->start_element) (context,
                                        start_name,
                                        (const char **)attr_names,
                                        (const char **)attr_values,
                                        context->user_data);
  free (attr_names);
  free (attr_values);
  clear_attributes (context);
}

static void
emit_end_element (markup_parse_context_ty *context)
{
  assert (gl_list_size (context->tag_stack) != 0);

  possibly_finish_subparser (context);

  /* We might have just returned from our ignore subparser */
  if ((context->flags & MARKUP_IGNORE_QUALIFIED)
      && strchr (current_element (context), ':'))
    {
      markup_parse_context_pop (context);
      pop_tag (context);
      return;
    }

  if (context->parser->end_element)
    (* context->parser->end_element) (context,
                                      current_element (context),
                                      context->user_data);

  ensure_no_outstanding_subparser (context);

  pop_tag (context);
}

/* Feed some data to the parse context.  The data need not be valid
   UTF-8; an error will be signaled if it's invalid.  The data need
   not be an entire document; you can feed a document into the parser
   incrementally, via multiple calls to this function.  Typically, as
   you receive data from a network connection or file, you feed each
   received chunk of data into this function, aborting the process if
   an error occurs. Once an error is reported, no further data may be
   fed to the parse context; all errors are fatal.  */
bool
markup_parse_context_parse (markup_parse_context_ty *context,
                            const char *text,
                            ssize_t text_len)
{
  assert (context != NULL);
  assert (text != NULL);
  assert (context->state != STATE_ERROR);
  assert (!context->parsing);

  if (text_len < 0)
    text_len = strlen (text);

  if (text_len == 0)
    return true;

  context->parsing = true;


  context->current_text = text;
  context->current_text_len = text_len;
  context->current_text_end = context->current_text + text_len;
  context->iter = context->current_text;
  context->start = context->iter;

  while (context->iter != context->current_text_end)
    {
      switch (context->state)
        {
        case STATE_START:
          /* Possible next state: AFTER_OPEN_ANGLE */

          assert (gl_list_size (context->tag_stack) == 0);

          /* whitespace is ignored outside of any elements */
          skip_spaces (context);

          if (context->iter != context->current_text_end)
            {
              if (*context->iter == '<')
                {
                  /* Move after the open angle */
                  advance_char (context);

                  context->state = STATE_AFTER_OPEN_ANGLE;

                  /* this could start a passthrough */
                  context->start = context->iter;

                  /* document is now non-empty */
                  context->document_empty = false;
                }
              else
                {
                  emit_error (context,
                              _("document must begin with an element"));
                }
            }
          break;

        case STATE_AFTER_OPEN_ANGLE:
          /* Possible next states: INSIDE_OPEN_TAG_NAME,
           *  AFTER_CLOSE_TAG_SLASH, INSIDE_PASSTHROUGH
           */
          if (*context->iter == '?' ||
              *context->iter == '!')
            {
              /* include < in the passthrough */
              const char *openangle = "<";
              add_to_partial (context, openangle, openangle + 1);
              context->start = context->iter;
              context->balance = 1;
              context->state = STATE_INSIDE_PASSTHROUGH;
            }
          else if (*context->iter == '/')
            {
              /* move after it */
              advance_char (context);

              context->state = STATE_AFTER_CLOSE_TAG_SLASH;
            }
          else if (!IS_COMMON_NAME_END_CHAR (*(context->iter)))
            {
              context->state = STATE_INSIDE_OPEN_TAG_NAME;

              /* start of tag name */
              context->start = context->iter;
            }
          else
            {
              char *error_text = xasprintf (_("invalid character after '%s'"),
                                            "<");
              emit_error (context, error_text);
              free (error_text);
            }
          break;

          /* The AFTER_CLOSE_ANGLE state is actually sort of
           * broken, because it doesn't correspond to a range
           * of characters in the input stream as the others do,
           * and thus makes things harder to conceptualize
           */
        case STATE_AFTER_CLOSE_ANGLE:
          /* Possible next states: INSIDE_TEXT, STATE_START */
          if (gl_list_size (context->tag_stack) == 0)
            {
              context->start = NULL;
              context->state = STATE_START;
            }
          else
            {
              context->start = context->iter;
              context->state = STATE_INSIDE_TEXT;
            }
          break;

        case STATE_AFTER_ELISION_SLASH:
          /* Possible next state: AFTER_CLOSE_ANGLE */
          if (*context->iter == '>')
            {
              /* move after the close angle */
              advance_char (context);
              context->state = STATE_AFTER_CLOSE_ANGLE;
              emit_end_element (context);
            }
          else
            {
              char *error_text = xasprintf (_("missing '%c'"), '>');
              emit_error (context, error_text);
              free (error_text);
            }
          break;

        case STATE_INSIDE_OPEN_TAG_NAME:
          /* Possible next states: BETWEEN_ATTRIBUTES */

          /* if there's a partial chunk then it's the first part of the
           * tag name. If there's a context->start then it's the start
           * of the tag name in current_text, the partial chunk goes
           * before that start though.
           */
          advance_to_name_end (context);

          if (context->iter == context->current_text_end)
            {
              /* The name hasn't necessarily ended. Merge with
               * partial chunk, leave state unchanged.
               */
              add_to_partial (context, context->start, context->iter);
            }
          else
            {
              /* The name has ended. Combine it with the partial chunk
               * if any; push it on the stack; enter next state.
               */
              add_to_partial (context, context->start, context->iter);
              push_partial_as_tag (context);

              context->state = STATE_BETWEEN_ATTRIBUTES;
              context->start = NULL;
            }
          break;

        case STATE_INSIDE_ATTRIBUTE_NAME:
          /* Possible next states: AFTER_ATTRIBUTE_NAME */

          advance_to_name_end (context);
          add_to_partial (context, context->start, context->iter);

          /* read the full name, if we enter the equals sign state
           * then add the attribute to the list (without the value),
           * otherwise store a partial chunk to be prepended later.
           */
          if (context->iter != context->current_text_end)
            context->state = STATE_AFTER_ATTRIBUTE_NAME;
          break;

        case STATE_AFTER_ATTRIBUTE_NAME:
          /* Possible next states: AFTER_ATTRIBUTE_EQUALS_SIGN */

          skip_spaces (context);

          if (context->iter != context->current_text_end)
            {
              /* The name has ended. Combine it with the partial chunk
               * if any; push it on the stack; enter next state.
               */
              if (!name_validate (context, context->partial_chunk->buffer))
                break;

              add_attribute (context, context->partial_chunk);

              markup_string_free (context->partial_chunk, true);
              context->partial_chunk = NULL;
              context->start = NULL;

              if (*context->iter == '=')
                {
                  advance_char (context);
                  context->state = STATE_AFTER_ATTRIBUTE_EQUALS_SIGN;
                }
              else
                {
                  char *error_text = xasprintf (_("missing '%c'"), '=');
                  emit_error (context, error_text);
                  free (error_text);
                }
            }
          break;

        case STATE_BETWEEN_ATTRIBUTES:
          /* Possible next states: AFTER_CLOSE_ANGLE,
           * AFTER_ELISION_SLASH, INSIDE_ATTRIBUTE_NAME
           */
          skip_spaces (context);

          if (context->iter != context->current_text_end)
            {
              if (*context->iter == '/')
                {
                  advance_char (context);
                  context->state = STATE_AFTER_ELISION_SLASH;
                }
              else if (*context->iter == '>')
                {
                  advance_char (context);
                  context->state = STATE_AFTER_CLOSE_ANGLE;
                }
              else if (!IS_COMMON_NAME_END_CHAR (*(context->iter)))
                {
                  context->state = STATE_INSIDE_ATTRIBUTE_NAME;
                  /* start of attribute name */
                  context->start = context->iter;
                }
              else
                {
                  char *error_text = xasprintf (_("missing '%c' or '%c'"),
                                                '>', '/');
                  emit_error (context, error_text);
                  free (error_text);
                }

              /* If we're done with attributes, invoke
               * the start_element callback
               */
              if (context->state == STATE_AFTER_ELISION_SLASH ||
                  context->state == STATE_AFTER_CLOSE_ANGLE)
                emit_start_element (context);
            }
          break;

        case STATE_AFTER_ATTRIBUTE_EQUALS_SIGN:
          /* Possible next state: INSIDE_ATTRIBUTE_VALUE_[SQ/DQ] */

          skip_spaces (context);

          if (context->iter != context->current_text_end)
            {
              if (*context->iter == '"')
                {
                  advance_char (context);
                  context->state = STATE_INSIDE_ATTRIBUTE_VALUE_DQ;
                  context->start = context->iter;
                }
              else if (*context->iter == '\'')
                {
                  advance_char (context);
                  context->state = STATE_INSIDE_ATTRIBUTE_VALUE_SQ;
                  context->start = context->iter;
                }
              else
                {
                  char *error_text = xasprintf (_("missing '%c' or '%c'"),
                                                '\'', '"');
                  emit_error (context, error_text);
                  free (error_text);
                }
            }
          break;

        case STATE_INSIDE_ATTRIBUTE_VALUE_SQ:
        case STATE_INSIDE_ATTRIBUTE_VALUE_DQ:
          /* Possible next states: BETWEEN_ATTRIBUTES */
          {
            char delim;

            if (context->state == STATE_INSIDE_ATTRIBUTE_VALUE_SQ)
              {
                delim = '\'';
              }
            else
              {
                delim = '"';
              }

            do
              {
                if (*context->iter == delim)
                  break;
              }
            while (advance_char (context));
          }
          if (context->iter == context->current_text_end)
            {
              /* The value hasn't necessarily ended. Merge with
               * partial chunk, leave state unchanged.
               */
              add_to_partial (context, context->start, context->iter);
            }
          else
            {
              bool is_ascii;
              /* The value has ended at the quote mark. Combine it
               * with the partial chunk if any; set it for the current
               * attribute.
               */
              add_to_partial (context, context->start, context->iter);

              assert (context->cur_attr >= 0);

              if (unescape_string_inplace (context, context->partial_chunk,
                                           &is_ascii)
                  && (is_ascii
                      || text_validate (context,
                                        context->partial_chunk->buffer,
                                        context->partial_chunk->buflen)))
                {
                  /* success, advance past quote and set state. */
                  context->attr_values[context->cur_attr] =
                    markup_string_free (context->partial_chunk, false);
                  context->partial_chunk = NULL;
                  advance_char (context);
                  context->state = STATE_BETWEEN_ATTRIBUTES;
                  context->start = NULL;
                }

              truncate_partial (context);
            }
          break;

        case STATE_INSIDE_TEXT:
          /* Possible next states: AFTER_OPEN_ANGLE */
          do
            {
              if (*context->iter == '<')
                break;
            }
          while (advance_char (context));

          /* The text hasn't necessarily ended. Merge with
           * partial chunk, leave state unchanged.
           */

          add_to_partial (context, context->start, context->iter);

          if (context->iter != context->current_text_end)
            {
              bool is_ascii;

              /* The text has ended at the open angle. Call the text
               * callback.
               */
              if (unescape_string_inplace (context, context->partial_chunk,
                                           &is_ascii)
                  && (is_ascii
                      || text_validate (context,
                                        context->partial_chunk->buffer,
                                        context->partial_chunk->buflen)))
                {
                  if (context->parser->text)
                    (*context->parser->text) (context,
                                              context->partial_chunk->buffer,
                                              context->partial_chunk->buflen,
                                              context->user_data);

                  /* advance past open angle and set state. */
                  advance_char (context);
                  context->state = STATE_AFTER_OPEN_ANGLE;
                  /* could begin a passthrough */
                  context->start = context->iter;
                }

              truncate_partial (context);
            }
          break;

        case STATE_AFTER_CLOSE_TAG_SLASH:
          /* Possible next state: INSIDE_CLOSE_TAG_NAME */
          if (!IS_COMMON_NAME_END_CHAR (*(context->iter)))
            {
              context->state = STATE_INSIDE_CLOSE_TAG_NAME;

              /* start of tag name */
              context->start = context->iter;
            }
          else
            {
              char *error_text = xasprintf (_("invalid character after '%s'"),
                                            "</");
              emit_error (context, error_text);
              free (error_text);
            }
          break;

        case STATE_INSIDE_CLOSE_TAG_NAME:
          /* Possible next state: AFTER_CLOSE_TAG_NAME */
          advance_to_name_end (context);
          add_to_partial (context, context->start, context->iter);

          if (context->iter != context->current_text_end)
            context->state = STATE_AFTER_CLOSE_TAG_NAME;
          break;

        case STATE_AFTER_CLOSE_TAG_NAME:
          /* Possible next state: AFTER_CLOSE_TAG_SLASH */

          skip_spaces (context);

          if (context->iter != context->current_text_end)
            {
              markup_string_ty *close_name;

              close_name = context->partial_chunk;
              context->partial_chunk = NULL;

              if (*context->iter != '>')
                {
                  char *error_text =
                    xasprintf (_("invalid character after '%s'"),
                               _("a close element name"));
                  emit_error (context, error_text);
                  free (error_text);
                }
              else if (gl_list_size (context->tag_stack) == 0)
                {
                  emit_error (context, _("element is closed"));
                }
              else if (strcmp (close_name->buffer, current_element (context))
                       != 0)
                {
                  emit_error (context, _("element is closed"));
                }
              else
                {
                  advance_char (context);
                  context->state = STATE_AFTER_CLOSE_ANGLE;
                  context->start = NULL;

                  emit_end_element (context);
                }
              context->partial_chunk = close_name;
              truncate_partial (context);
            }
          break;

        case STATE_INSIDE_PASSTHROUGH:
          /* Possible next state: AFTER_CLOSE_ANGLE */
          do
            {
              if (*context->iter == '<')
                context->balance++;
              if (*context->iter == '>')
                {
                  char *str;
                  size_t len;

                  context->balance--;
                  add_to_partial (context, context->start, context->iter);
                  context->start = context->iter;

                  str = context->partial_chunk->buffer;
                  len = context->partial_chunk->buflen;

                  if (str[1] == '?' && str[len - 1] == '?')
                    break;
                  if (strncmp (str, "<!--", 4) == 0 &&
                      strcmp (str + len - 2, "--") == 0)
                    break;
                  if (strncmp (str, "<![CDATA[", 9) == 0 &&
                      strcmp (str + len - 2, "]]") == 0)
                    break;
                  if (strncmp (str, "<!DOCTYPE", 9) == 0 &&
                      context->balance == 0)
                    break;
                }
            }
          while (advance_char (context));

          if (context->iter == context->current_text_end)
            {
              /* The passthrough hasn't necessarily ended. Merge with
               * partial chunk, leave state unchanged.
               */
               add_to_partial (context, context->start, context->iter);
            }
          else
            {
              /* The passthrough has ended at the close angle. Combine
               * it with the partial chunk if any. Call the passthrough
               * callback. Note that the open/close angles are
               * included in the text of the passthrough.
               */
              advance_char (context); /* advance past close angle */
              add_to_partial (context, context->start, context->iter);

              if (context->flags & MARKUP_TREAT_CDATA_AS_TEXT &&
                  strncmp (context->partial_chunk->buffer, "<![CDATA[", 9) == 0)
                {
                  if (context->parser->text &&
                      text_validate (context,
                                     context->partial_chunk->buffer + 9,
                                     context->partial_chunk->buflen - 12))
                    (*context->parser->text) (context,
                                              context->partial_chunk->buffer + 9,
                                              context->partial_chunk->buflen - 12,
                                              context->user_data);
                }
              else if (context->parser->passthrough &&
                       text_validate (context,
                                      context->partial_chunk->buffer,
                                      context->partial_chunk->buflen))
                (*context->parser->passthrough) (context,
                                                 context->partial_chunk->buffer,
                                                 context->partial_chunk->buflen,
                                                 context->user_data);

              truncate_partial (context);

              context->state = STATE_AFTER_CLOSE_ANGLE;
              context->start = context->iter; /* could begin text */
            }
          break;

        case STATE_ERROR:
          goto finished;
          break;

        default:
          abort ();
          break;
        }
    }

 finished:
  context->parsing = false;

  return context->state != STATE_ERROR;
}

/* Signals to the parse context that all data has been fed into the
 * parse context with markup_parse_context_parse.
 *
 * This function reports an error if the document isn't complete,
 * for example if elements are still open.  */
bool
markup_parse_context_end_parse (markup_parse_context_ty *context)
{
  const char *location = NULL;

  assert (context != NULL);
  assert (!context->parsing);
  assert (context->state != STATE_ERROR);

  if (context->partial_chunk != NULL)
    {
      markup_string_free (context->partial_chunk, true);
      context->partial_chunk = NULL;
    }

  if (context->document_empty)
    {
      emit_error (context, _("empty document"));
      return false;
    }

  context->parsing = true;

  switch (context->state)
    {
    case STATE_START:
      /* Nothing to do */
      break;

    case STATE_AFTER_OPEN_ANGLE:
      location = _("after '<'");
      break;

    case STATE_AFTER_CLOSE_ANGLE:
      if (gl_list_size (context->tag_stack) > 0)
        {
          /* Error message the same as for INSIDE_TEXT */
          location = _("elements still open");
        }
      break;

    case STATE_AFTER_ELISION_SLASH:
      location = _("missing '>'");
      break;

    case STATE_INSIDE_OPEN_TAG_NAME:
      location = _("inside an element name");
      break;

    case STATE_INSIDE_ATTRIBUTE_NAME:
    case STATE_AFTER_ATTRIBUTE_NAME:
      location = _("inside an attribute name");
      break;

    case STATE_BETWEEN_ATTRIBUTES:
      location = _("inside an open tag");
      break;

    case STATE_AFTER_ATTRIBUTE_EQUALS_SIGN:
      location = _("after '='");
      break;

    case STATE_INSIDE_ATTRIBUTE_VALUE_SQ:
    case STATE_INSIDE_ATTRIBUTE_VALUE_DQ:
      location = _("inside an attribute value");
      break;

    case STATE_INSIDE_TEXT:
      assert (gl_list_size (context->tag_stack) > 0);
      location = _("elements still open");
      break;

    case STATE_AFTER_CLOSE_TAG_SLASH:
    case STATE_INSIDE_CLOSE_TAG_NAME:
    case STATE_AFTER_CLOSE_TAG_NAME:
      location = _("inside the close tag");
      break;

    case STATE_INSIDE_PASSTHROUGH:
      location = _("inside a comment or processing instruction");
      break;

    case STATE_ERROR:
    default:
      abort ();
      break;
    }

  if (location != NULL)
    {
      char *error_text = xasprintf (_("document ended unexpectedly: %s"),
                                    location);
      emit_error (context, error_text);
      free (error_text);
    }

  context->parsing = false;

  return context->state != STATE_ERROR;
}

const char *
markup_parse_context_get_error (markup_parse_context_ty *context)
{
  return context->error_text;
}
