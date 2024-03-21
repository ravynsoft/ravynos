/* Internationalization Tag Set (ITS) handling
   Copyright (C) 2015, 2018-2020, 2023 Free Software Foundation, Inc.

   This file was written by Daiki Ueno <ueno@gnu.org>, 2015.

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
#include <config.h>
#endif

/* Specification.  */
#include "its.h"

#include <assert.h>
#include <errno.h>
#include "error.h"
#include "gettext.h"
#include "mem-hash-map.h"
#include <stdint.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdlib.h>
#include "trim.h"
#include "xalloc.h"
#include "xvasprintf.h"

#define _(str) gettext (str)

/* The Internationalization Tag Set (ITS) 2.0 standard is available at:
   https://www.w3.org/TR/its20/

   This implementation supports only a few data categories, useful for
   gettext-based projects.  Other data categories can be added by
   extending the its_rule_class_ty class and registering it in
   init_classes().

   The message extraction is performed in three steps.  In the first
   step, its_rule_list_apply() assigns values to nodes in an XML
   document.  In the second step, its_rule_list_extract_nodes() marks
   translatable nodes.  In the final step,
   its_rule_list_extract_text() extracts text contents from the marked
   nodes.

   The values assigned to a node are represented as an array of
   key-value pairs, where both keys and values are string.  The array
   is stored in node->_private.  To retrieve the values for a node,
   use its_rule_list_eval().  */

#define ITS_NS "http://www.w3.org/2005/11/its"
#define XML_NS "http://www.w3.org/XML/1998/namespace"
#define GT_NS "https://www.gnu.org/s/gettext/ns/its/extensions/1.0"

struct its_value_ty
{
  char *name;
  char *value;
};

struct its_value_list_ty
{
  struct its_value_ty *items;
  size_t nitems;
  size_t nitems_max;
};

static void
its_value_list_append (struct its_value_list_ty *values,
                       const char *name,
                       const char *value)
{
  struct its_value_ty _value;

  _value.name = xstrdup (name);
  _value.value = xstrdup (value);

  if (values->nitems == values->nitems_max)
    {
      values->nitems_max = 2 * values->nitems_max + 1;
      values->items =
        xrealloc (values->items,
                  sizeof (struct its_value_ty) * values->nitems_max);
    }
  memcpy (&values->items[values->nitems++], &_value,
          sizeof (struct its_value_ty));
}

static const char *
its_value_list_get_value (struct its_value_list_ty *values,
                          const char *name)
{
  size_t i;

  for (i = 0; i < values->nitems; i++)
    {
      struct its_value_ty *value = &values->items[i];
      if (strcmp (value->name, name) == 0)
        return value->value;
    }
  return NULL;
}

static void
its_value_list_set_value (struct its_value_list_ty *values,
                          const char *name,
                          const char *value)
{
  size_t i;

  for (i = 0; i < values->nitems; i++)
    {
      struct its_value_ty *_value = &values->items[i];
      if (strcmp (_value->name, name) == 0)
        {
          free (_value->value);
          _value->value = xstrdup (value);
          break;
        }
    }

  if (i == values->nitems)
    its_value_list_append (values, name, value);
}

static void
its_value_list_merge (struct its_value_list_ty *values,
                      struct its_value_list_ty *other)
{
  size_t i;

  for (i = 0; i < other->nitems; i++)
    {
      struct its_value_ty *other_value = &other->items[i];
      size_t j;

      for (j = 0; j < values->nitems; j++)
        {
          struct its_value_ty *value = &values->items[j];

          if (strcmp (value->name, other_value->name) == 0
              && strcmp (value->value, other_value->value) != 0)
            {
              free (value->value);
              value->value = xstrdup (other_value->value);
              break;
            }
        }

      if (j == values->nitems)
        its_value_list_append (values, other_value->name, other_value->value);
    }
}

static void
its_value_list_destroy (struct its_value_list_ty *values)
{
  size_t i;

  for (i = 0; i < values->nitems; i++)
    {
      free (values->items[i].name);
      free (values->items[i].value);
    }
  free (values->items);
}

struct its_pool_ty
{
  struct its_value_list_ty *items;
  size_t nitems;
  size_t nitems_max;
};

static struct its_value_list_ty *
its_pool_alloc_value_list (struct its_pool_ty *pool)
{
  struct its_value_list_ty *values;

  if (pool->nitems == pool->nitems_max)
    {
      pool->nitems_max = 2 * pool->nitems_max + 1;
      pool->items =
        xrealloc (pool->items,
                  sizeof (struct its_value_list_ty) * pool->nitems_max);
    }

  values = &pool->items[pool->nitems++];
  memset (values, 0, sizeof (struct its_value_list_ty));
  return values;
}

static const char *
its_pool_get_value_for_node (struct its_pool_ty *pool, xmlNode *node,
                              const char *name)
{
  intptr_t index = (intptr_t) node->_private;
  if (index > 0)
    {
      struct its_value_list_ty *values;

      assert (index <= pool->nitems);
      values = &pool->items[index - 1];

      return its_value_list_get_value (values, name);
    }
  return NULL;
}

static void
its_pool_destroy (struct its_pool_ty *pool)
{
  size_t i;

  for (i = 0; i < pool->nitems; i++)
    its_value_list_destroy (&pool->items[i]);
  free (pool->items);
}

struct its_rule_list_ty
{
  struct its_rule_ty **items;
  size_t nitems;
  size_t nitems_max;

  struct its_pool_ty pool;
};

struct its_node_list_ty
{
  xmlNode **items;
  size_t nitems;
  size_t nitems_max;
};

static void
its_node_list_append (struct its_node_list_ty *nodes,
                      xmlNode *node)
{
  if (nodes->nitems == nodes->nitems_max)
    {
      nodes->nitems_max = 2 * nodes->nitems_max + 1;
      nodes->items =
        xrealloc (nodes->items, sizeof (xmlNode *) * nodes->nitems_max);
    }
  nodes->items[nodes->nitems++] = node;
}

/* Base class representing an ITS rule in global definition.  */
struct its_rule_class_ty
{
  /* How many bytes to malloc for an instance of this class.  */
  size_t size;

  /* What to do immediately after the instance is malloc()ed.  */
  void (*constructor) (struct its_rule_ty *pop, xmlNode *node);

  /* What to do immediately before the instance is free()ed.  */
  void (*destructor) (struct its_rule_ty *pop);

  /* How to apply the rule to all elements in DOC.  */
  void (* apply) (struct its_rule_ty *pop, struct its_pool_ty *pool,
                  xmlDoc *doc);

  /* How to evaluate the value of NODE according to the rule.  */
  struct its_value_list_ty *(* eval) (struct its_rule_ty *pop,
                                      struct its_pool_ty *pool, xmlNode *node);
};

#define ITS_RULE_TY                             \
  struct its_rule_class_ty *methods;            \
  char *selector;                               \
  struct its_value_list_ty values;              \
  xmlNs **namespaces;

struct its_rule_ty
{
  ITS_RULE_TY
};

static hash_table classes;

static void
its_rule_destructor (struct its_rule_ty *pop)
{
  free (pop->selector);
  its_value_list_destroy (&pop->values);
  if (pop->namespaces)
    {
      size_t i;
      for (i = 0; pop->namespaces[i] != NULL; i++)
        xmlFreeNs (pop->namespaces[i]);
      free (pop->namespaces);
    }
}

static void
its_rule_apply (struct its_rule_ty *rule, struct its_pool_ty *pool, xmlDoc *doc)
{
  xmlXPathContext *context;
  xmlXPathObject *object;

  if (!rule->selector)
    {
      error (0, 0, _("selector is not specified"));
      return;
    }

  context = xmlXPathNewContext (doc);
  if (!context)
    {
      error (0, 0, _("cannot create XPath context"));
      return;
    }

  if (rule->namespaces)
    {
      size_t i;
      for (i = 0; rule->namespaces[i] != NULL; i++)
        {
          xmlNs *ns = rule->namespaces[i];
          xmlXPathRegisterNs (context, ns->prefix, ns->href);
        }
    }

  object = xmlXPathEval (BAD_CAST rule->selector, context);
  if (!object)
    {
      xmlXPathFreeContext (context);
      error (0, 0, _("cannot evaluate XPath expression: %s"), rule->selector);
      return;
    }

  if (object->nodesetval)
    {
      xmlNodeSet *nodes = object->nodesetval;
      size_t i;

      for (i = 0; i < nodes->nodeNr; i++)
        {
          xmlNode *node = nodes->nodeTab[i];
          struct its_value_list_ty *values;

          /* We can't store VALUES in NODE, since the address can
             change when realloc()ed.  */
          intptr_t index = (intptr_t) node->_private;

          assert (index <= pool->nitems);
          if (index > 0)
            values = &pool->items[index - 1];
          else
            {
              values = its_pool_alloc_value_list (pool);
              node->_private = (void *) pool->nitems;
            }

          its_value_list_merge (values, &rule->values);
        }
    }

  xmlXPathFreeObject (object);
  xmlXPathFreeContext (context);
}

static char *
_its_get_attribute (xmlNode *node, const char *attr, const char *namespace)
{
  xmlChar *value;
  char *result;

  value = xmlGetNsProp (node, BAD_CAST attr, BAD_CAST namespace);

  result = xstrdup ((const char *) value);
  xmlFree (value);

  return result;
}

static char *
normalize_whitespace (const char *text, enum its_whitespace_type_ty whitespace)
{
  switch (whitespace)
    {
    case ITS_WHITESPACE_PRESERVE:
      return xstrdup (text);

    case ITS_WHITESPACE_TRIM:
      return trim (text);

    case ITS_WHITESPACE_NORMALIZE_PARAGRAPH:
      /* Normalize whitespaces within the text, keeping paragraph
         boundaries.  */
      {
        char *result = xstrdup (text);
        /* Go through the string, shrinking it, reading from *p++
           and writing to *out++.  (result <= out <= p.)  */
        const char *start_of_paragraph;
        char *out;

        out = result;
        for (start_of_paragraph = result; *start_of_paragraph != '\0';)
          {
            const char *end_of_paragraph;
            const char *next_paragraph;

            /* Find the next paragraph boundary.  */
            {
              const char *p;

              for (p = start_of_paragraph;;)
                {
                  const char *nl = strchrnul (p, '\n');
                  if (*nl == '\0')
                    {
                      end_of_paragraph = nl;
                      next_paragraph = end_of_paragraph;
                      break;
                    }
                  p = nl + 1;
                  {
                    const char *past_whitespace = p + strspn (p, " \t\n");
                    if (memchr (p, '\n', past_whitespace - p) != NULL)
                      {
                        end_of_paragraph = nl;
                        next_paragraph = past_whitespace;
                        break;
                      }
                    p = past_whitespace;
                  }
                }
            }

            /* Normalize whitespaces in the paragraph.  */
            {
              const char *p;

              /* Remove whitespace at the beginning of the paragraph.  */
              for (p = start_of_paragraph; p < end_of_paragraph; p++)
                if (!(*p == ' ' || *p == '\t' || *p == '\n'))
                  break;

              for (; p < end_of_paragraph;)
                {
                  if (*p == ' ' || *p == '\t' || *p == '\n')
                    {
                      /* Normalize whitespace inside the paragraph, and
                         remove whitespace at the end of the paragraph.  */
                      do
                        p++;
                      while (p < end_of_paragraph
                             && (*p == ' ' || *p == '\t' || *p == '\n'));
                      if (p < end_of_paragraph)
                        *out++ = ' ';
                    }
                  else
                    *out++ = *p++;
                }
            }

            if (*next_paragraph != '\0')
              {
                memcpy (out, "\n\n", 2);
                out += 2;
              }
            start_of_paragraph = next_paragraph;
          }
        *out = '\0';
        return result;
      }
    default:
      /* Normalize whitespaces within the text, but do not eliminate whitespace
         at the beginning nor the end of the text.  */
      {
        char *result = xstrdup (text);
        char *out;
        const char *p;

        out = result;
        for (p = result; *p != '\0';)
          {
            if (*p == ' ' || *p == '\t' || *p == '\n')
              {
                do
                  p++;
                while (*p == ' ' || *p == '\t' || *p == '\n');
                *out++ = ' ';
              }
            else
              *out++ = *p++;
          }
        *out = '\0';
        return result;
      }
    }
}

static char *
_its_encode_special_chars (const char *content, bool is_attribute)
{
  const char *str;
  size_t amount = 0;
  char *result, *p;

  for (str = content; *str != '\0'; str++)
    {
      switch (*str)
        {
        case '&':
          amount += sizeof ("&amp;");
          break;
        case '<':
          amount += sizeof ("&lt;");
          break;
        case '>':
          amount += sizeof ("&gt;");
          break;
        case '"':
          if (is_attribute)
            amount += sizeof ("&quot;");
          else
            amount += 1;
          break;
        default:
          amount += 1;
          break;
        }
    }

  result = XNMALLOC (amount + 1, char);
  *result = '\0';
  p = result;
  for (str = content; *str != '\0'; str++)
    {
      switch (*str)
        {
        case '&':
          p = stpcpy (p, "&amp;");
          break;
        case '<':
          p = stpcpy (p, "&lt;");
          break;
        case '>':
          p = stpcpy (p, "&gt;");
          break;
        case '"':
          if (is_attribute)
            p = stpcpy (p, "&quot;");
          else
            *p++ = '"';
          break;
        default:
          *p++ = *str;
          break;
        }
    }
  *p = '\0';
  return result;
}

static char *
_its_collect_text_content (xmlNode *node,
                           enum its_whitespace_type_ty whitespace,
                           bool no_escape)
{
  char *buffer = NULL;
  size_t bufmax = 0;
  size_t bufpos = 0;
  xmlNode *n;

  for (n = node->children; n; n = n->next)
    {
      char *content = NULL;

      switch (n->type)
        {
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
          {
            xmlChar *xcontent = xmlNodeGetContent (n);
            char *econtent;
            const char *ccontent;

            /* We can't expect xmlTextWriterWriteString() encode
               special characters as we write text outside of the
               element.  */
            if (no_escape)
              econtent = xstrdup ((const char *) xcontent);
            else
              econtent =
                _its_encode_special_chars ((const char *) xcontent,
                                           node->type == XML_ATTRIBUTE_NODE);
            xmlFree (xcontent);

            /* Skip whitespaces at the beginning of the text, if this
               is the first node.  */
            ccontent = econtent;
            if (whitespace == ITS_WHITESPACE_NORMALIZE && !n->prev)
              ccontent = ccontent + strspn (ccontent, " \t\n");
            content =
              normalize_whitespace (ccontent, whitespace);
            free (econtent);

            /* Skip whitespaces at the end of the text, if this
               is the last node.  */
            if (whitespace == ITS_WHITESPACE_NORMALIZE && !n->next)
              {
                char *p = content + strlen (content);
                for (; p > content; p--)
                  {
                    int c = *(p - 1);
                    if (!(c == ' ' || c == '\t' || c == '\n'))
                      {
                        *p = '\0';
                        break;
                      }
                  }
              }
          }
          break;

        case XML_ELEMENT_NODE:
          {
            xmlOutputBuffer *obuffer = xmlAllocOutputBuffer (NULL);
            xmlTextWriter *writer = xmlNewTextWriter (obuffer);
            char *p = _its_collect_text_content (n, whitespace,
                                                 no_escape);
            const char *ccontent;

            xmlTextWriterStartElement (writer, BAD_CAST n->name);
            if (n->properties)
              {
                xmlAttr *attr = n->properties;
                for (; attr; attr = attr->next)
                  {
                    xmlChar *prop = xmlGetProp (n, attr->name);
                    xmlTextWriterWriteAttribute (writer,
                                                 attr->name,
                                                 prop);
                    xmlFree (prop);
                  }
              }
            if (*p != '\0')
              xmlTextWriterWriteRaw (writer, BAD_CAST p);
            xmlTextWriterEndElement (writer);
            ccontent = (const char *) xmlOutputBufferGetContent (obuffer);
            content = normalize_whitespace (ccontent, whitespace);
            xmlFreeTextWriter (writer);
            free (p);
          }
          break;

        case XML_ENTITY_REF_NODE:
          content = xasprintf ("&%s;", (const char *) n->name);
          break;

        default:
          break;
        }

      if (content != NULL)
        {
          size_t length = strlen (content);

          if (bufpos + length + 1 >= bufmax)
            {
              bufmax = 2 * bufmax + length + 1;
              buffer = xrealloc (buffer, bufmax);
            }
          strcpy (&buffer[bufpos], content);
          bufpos += length;
        }
      free (content);
    }

  if (buffer == NULL)
    buffer = xstrdup ("");
  return buffer;
}

static void
_its_error_missing_attribute (xmlNode *node, const char *attribute)
{
  error (0, 0, _("\"%s\" node does not contain \"%s\""),
         node->name, attribute);
}

/* Implementation of Translate data category.  */
static void
its_translate_rule_constructor (struct its_rule_ty *pop, xmlNode *node)
{
  char *prop;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "translate"))
    {
      _its_error_missing_attribute (node, "translate");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  prop = _its_get_attribute (node, "translate", NULL);
  its_value_list_append (&pop->values, "translate", prop);
  free (prop);
}

static struct its_value_list_ty *
its_translate_rule_eval (struct its_rule_ty *pop, struct its_pool_ty *pool,
                         xmlNode *node)
{
  struct its_value_list_ty *result;

  result = XCALLOC (1, struct its_value_list_ty);

  switch (node->type)
    {
    case XML_ATTRIBUTE_NODE:
      /* Attribute nodes don't inherit from the parent elements.  */
      {
        const char *value =
          its_pool_get_value_for_node (pool, node, "translate");
        if (value != NULL)
          {
            its_value_list_set_value (result, "translate", value);
            return result;
          }

        /* The default value is translate="no".  */
        its_value_list_append (result, "translate", "no");
      }
      break;

    case XML_ELEMENT_NODE:
      /* Inherit from the parent elements.  */
      {
        const char *value;

        /* A local attribute overrides the global rule.  */
        if (xmlHasNsProp (node, BAD_CAST "translate", BAD_CAST ITS_NS))
          {
            char *prop;

            prop = _its_get_attribute (node, "translate", ITS_NS);
            its_value_list_append (result, "translate", prop);
            free (prop);
            return result;
          }

        /* Check value for the current node.  */
        value = its_pool_get_value_for_node (pool, node, "translate");
        if (value != NULL)
          {
            its_value_list_set_value (result, "translate", value);
            return result;
          }

        /* Recursively check value for the parent node.  */
        if (node->parent == NULL
            || node->parent->type != XML_ELEMENT_NODE)
          /* The default value is translate="yes".  */
          its_value_list_append (result, "translate", "yes");
        else
          {
            struct its_value_list_ty *values;

            values = its_translate_rule_eval (pop, pool, node->parent);
            its_value_list_merge (result, values);
            its_value_list_destroy (values);
            free (values);
          }
      }
      break;

    default:
      break;
    }

  return result;
}

static struct its_rule_class_ty its_translate_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_translate_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_translate_rule_eval,
  };

/* Implementation of Localization Note data category.  */
static void
its_localization_note_rule_constructor (struct its_rule_ty *pop, xmlNode *node)
{
  char *prop;
  xmlNode *n;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "locNoteType"))
    {
      _its_error_missing_attribute (node, "locNoteType");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  for (n = node->children; n; n = n->next)
    {
      if (n->type == XML_ELEMENT_NODE
          && xmlStrEqual (n->name, BAD_CAST "locNote")
          && xmlStrEqual (n->ns->href, BAD_CAST ITS_NS))
        break;
    }

  prop = _its_get_attribute (node, "locNoteType", NULL);
  if (prop)
    its_value_list_append (&pop->values, "locNoteType", prop);
  free (prop);

  if (n)
    {
      /* FIXME: Respect space attribute.  */
      char *content = _its_collect_text_content (n, ITS_WHITESPACE_NORMALIZE,
                                                 false);
      its_value_list_append (&pop->values, "locNote", content);
      free (content);
    }
  else if (xmlHasProp (node, BAD_CAST "locNotePointer"))
    {
      prop = _its_get_attribute (node, "locNotePointer", NULL);
      its_value_list_append (&pop->values, "locNotePointer", prop);
      free (prop);
    }
  /* FIXME: locNoteRef and locNoteRefPointer */
}

static struct its_value_list_ty *
its_localization_note_rule_eval (struct its_rule_ty *pop,
                                 struct its_pool_ty *pool,
                                 xmlNode *node)
{
  struct its_value_list_ty *result;

  result = XCALLOC (1, struct its_value_list_ty);

  switch (node->type)
    {
    case XML_ATTRIBUTE_NODE:
      /* Attribute nodes don't inherit from the parent elements.  */
      {
        const char *value;

        value = its_pool_get_value_for_node (pool, node, "locNoteType");
        if (value != NULL)
          its_value_list_set_value (result, "locNoteType", value);

        value = its_pool_get_value_for_node (pool, node, "locNote");
        if (value != NULL)
          {
            its_value_list_set_value (result, "locNote", value);
            return result;
          }

        value = its_pool_get_value_for_node (pool, node, "locNotePointer");
        if (value != NULL)
          {
            its_value_list_set_value (result, "locNotePointer", value);
            return result;
          }
      }
      break;

    case XML_ELEMENT_NODE:
      /* Inherit from the parent elements.  */
      {
        const char *value;

        /* Local attributes overrides the global rule.  */
        if (xmlHasNsProp (node, BAD_CAST "locNote", BAD_CAST ITS_NS)
            || xmlHasNsProp (node, BAD_CAST "locNoteRef", BAD_CAST ITS_NS)
            || xmlHasNsProp (node, BAD_CAST "locNoteType", BAD_CAST ITS_NS))
          {
            char *prop;

            if (xmlHasNsProp (node, BAD_CAST "locNote", BAD_CAST ITS_NS))
              {
                prop = _its_get_attribute (node, "locNote", ITS_NS);
                its_value_list_append (result, "locNote", prop);
                free (prop);
              }

            /* FIXME: locNoteRef */

            if (xmlHasNsProp (node, BAD_CAST "locNoteType", BAD_CAST ITS_NS))
              {
                prop = _its_get_attribute (node, "locNoteType", ITS_NS);
                its_value_list_append (result, "locNoteType", prop);
                free (prop);
              }

            return result;
          }

        /* Check value for the current node.  */
        value = its_pool_get_value_for_node (pool, node, "locNoteType");
        if (value != NULL)
          its_value_list_set_value (result, "locNoteType", value);

        value = its_pool_get_value_for_node (pool, node, "locNote");
        if (value != NULL)
          {
            its_value_list_set_value (result, "locNote", value);
            return result;
          }

        value = its_pool_get_value_for_node (pool, node, "locNotePointer");
        if (value != NULL)
          {
            its_value_list_set_value (result, "locNotePointer", value);
            return result;
          }

        /* Recursively check value for the parent node.  */
        if (node->parent == NULL
            || node->parent->type != XML_ELEMENT_NODE)
          return result;
        else
          {
            struct its_value_list_ty *values;

            values = its_localization_note_rule_eval (pop, pool, node->parent);
            its_value_list_merge (result, values);
            its_value_list_destroy (values);
            free (values);
          }
      }
      break;

    default:
      break;
    }

  /* The default value is None.  */
  return result;
}

static struct its_rule_class_ty its_localization_note_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_localization_note_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_localization_note_rule_eval,
  };

/* Implementation of Element Within Text data category.  */
static void
its_element_within_text_rule_constructor (struct its_rule_ty *pop,
                                          xmlNode *node)
{
  char *prop;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "withinText"))
    {
      _its_error_missing_attribute (node, "withinText");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  prop = _its_get_attribute (node, "withinText", NULL);
  its_value_list_append (&pop->values, "withinText", prop);
  free (prop);
}

static struct its_value_list_ty *
its_element_within_text_rule_eval (struct its_rule_ty *pop,
                                   struct its_pool_ty *pool,
                                   xmlNode *node)
{
  struct its_value_list_ty *result;
  const char *value;

  result = XCALLOC (1, struct its_value_list_ty);

  if (node->type != XML_ELEMENT_NODE)
    return result;

  /* A local attribute overrides the global rule.  */
  if (xmlHasNsProp (node, BAD_CAST "withinText", BAD_CAST ITS_NS))
    {
      char *prop;

      prop = _its_get_attribute (node, "withinText", ITS_NS);
      its_value_list_append (result, "withinText", prop);
      free (prop);
      return result;
    }

  /* Doesn't inherit from the parent elements, and the default value
     is None.  */
  value = its_pool_get_value_for_node (pool, node, "withinText");
  if (value != NULL)
    its_value_list_set_value (result, "withinText", value);

  return result;
}

static struct its_rule_class_ty its_element_within_text_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_element_within_text_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_element_within_text_rule_eval,
  };

/* Implementation of Preserve Space data category.  */
static void
its_preserve_space_rule_constructor (struct its_rule_ty *pop,
                                     xmlNode *node)
{
  char *prop;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "space"))
    {
      _its_error_missing_attribute (node, "space");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  prop = _its_get_attribute (node, "space", NULL);
  if (prop
      && !(strcmp (prop, "preserve") ==0 
           || strcmp (prop, "default") == 0
           /* gettext extension: remove leading/trailing whitespaces only.  */
           || (node->ns && xmlStrEqual (node->ns->href, BAD_CAST GT_NS)
               && strcmp (prop, "trim") == 0)
           /* gettext extension: same as default except keeping
              paragraph boundaries.  */
           || (node->ns && xmlStrEqual (node->ns->href, BAD_CAST GT_NS)
               && strcmp (prop, "paragraph") == 0)))
    {
      error (0, 0, _("invalid attribute value \"%s\" for \"%s\""),
             prop, "space");
      free (prop);
      return;
    }

  its_value_list_append (&pop->values, "space", prop);
  free (prop);
}

static struct its_value_list_ty *
its_preserve_space_rule_eval (struct its_rule_ty *pop,
                              struct its_pool_ty *pool,
                              xmlNode *node)
{
  struct its_value_list_ty *result;
  struct its_value_list_ty *values;
  const char *value;

  result = XCALLOC (1, struct its_value_list_ty);

  if (node->type != XML_ELEMENT_NODE)
    return result;

  /* A local attribute overrides the global rule.  */
  if (xmlHasNsProp (node, BAD_CAST "space", BAD_CAST XML_NS))
    {
      char *prop;

      prop = _its_get_attribute (node, "space", XML_NS);
      its_value_list_append (result, "space", prop);
      free (prop);
      return result;
    }

  /* Check value for the current node.  */
  value = its_pool_get_value_for_node (pool, node, "space");
  if (value != NULL)
    {
      its_value_list_set_value (result, "space", value);
      return result;
    }

  if (node->parent == NULL
      || node->parent->type != XML_ELEMENT_NODE)
    {
      /* The default value is space="default".  */
      its_value_list_append (result, "space", "default");
      return result;
    }

  /* Recursively check value for the parent node.  */
  values = its_preserve_space_rule_eval (pop, pool, node->parent);
  its_value_list_merge (result, values);
  its_value_list_destroy (values);
  free (values);

  return result;
}

static struct its_rule_class_ty its_preserve_space_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_preserve_space_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_preserve_space_rule_eval,
  };

/* Implementation of Context data category.  */
static void
its_extension_context_rule_constructor (struct its_rule_ty *pop, xmlNode *node)
{
  char *prop;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "contextPointer"))
    {
      _its_error_missing_attribute (node, "contextPointer");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  prop = _its_get_attribute (node, "contextPointer", NULL);
  its_value_list_append (&pop->values, "contextPointer", prop);
  free (prop);

  if (xmlHasProp (node, BAD_CAST "textPointer"))
    {
      prop = _its_get_attribute (node, "textPointer", NULL);
      its_value_list_append (&pop->values, "textPointer", prop);
      free (prop);
    }
}

static struct its_value_list_ty *
its_extension_context_rule_eval (struct its_rule_ty *pop,
                                 struct its_pool_ty *pool,
                                 xmlNode *node)
{
  struct its_value_list_ty *result;
  const char *value;

  result = XCALLOC (1, struct its_value_list_ty);

  /* Doesn't inherit from the parent elements, and the default value
     is None.  */
  value = its_pool_get_value_for_node (pool, node, "contextPointer");
  if (value != NULL)
    its_value_list_set_value (result, "contextPointer", value);

  value = its_pool_get_value_for_node (pool, node, "textPointer");
  if (value != NULL)
    its_value_list_set_value (result, "textPointer", value);

  return result;
}

static struct its_rule_class_ty its_extension_context_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_extension_context_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_extension_context_rule_eval,
  };

/* Implementation of Escape Special Characters data category.  */
static void
its_extension_escape_rule_constructor (struct its_rule_ty *pop, xmlNode *node)
{
  char *prop;

  if (!xmlHasProp (node, BAD_CAST "selector"))
    {
      _its_error_missing_attribute (node, "selector");
      return;
    }

  if (!xmlHasProp (node, BAD_CAST "escape"))
    {
      _its_error_missing_attribute (node, "escape");
      return;
    }

  prop = _its_get_attribute (node, "selector", NULL);
  if (prop)
    pop->selector = prop;

  prop = _its_get_attribute (node, "escape", NULL);
  its_value_list_append (&pop->values, "escape", prop);
  free (prop);
}

static struct its_value_list_ty *
its_extension_escape_rule_eval (struct its_rule_ty *pop,
                                struct its_pool_ty *pool,
                                xmlNode *node)
{
  struct its_value_list_ty *result;

  result = XCALLOC (1, struct its_value_list_ty);

  switch (node->type)
    {
    case XML_ATTRIBUTE_NODE:
      /* Attribute nodes don't inherit from the parent elements.  */
      {
        const char *value =
          its_pool_get_value_for_node (pool, node, "escape");
        if (value != NULL)
          {
            its_value_list_set_value (result, "escape", value);
            return result;
          }
      }
      break;

    case XML_ELEMENT_NODE:
      /* Inherit from the parent elements.  */
      {
        const char *value;

        /* Check value for the current node.  */
        value = its_pool_get_value_for_node (pool, node, "escape");
        if (value != NULL)
          {
            its_value_list_set_value (result, "escape", value);
            return result;
          }

        /* Recursively check value for the parent node.  */
        if (node->parent != NULL
            && node->parent->type == XML_ELEMENT_NODE)
          {
            struct its_value_list_ty *values;

            values = its_extension_escape_rule_eval (pop, pool, node->parent);
            its_value_list_merge (result, values);
            its_value_list_destroy (values);
            free (values);
          }
      }
      break;

    default:
      break;
    }

  return result;
}

static struct its_rule_class_ty its_extension_escape_rule_class =
  {
    sizeof (struct its_rule_ty),
    its_extension_escape_rule_constructor,
    its_rule_destructor,
    its_rule_apply,
    its_extension_escape_rule_eval,
  };

static struct its_rule_ty *
its_rule_alloc (struct its_rule_class_ty *method_table, xmlNode *node)
{
  struct its_rule_ty *pop;

  pop = (struct its_rule_ty *) xcalloc (1, method_table->size);
  pop->methods = method_table;
  if (method_table->constructor)
    method_table->constructor (pop, node);
  return pop;
}

static struct its_rule_ty *
its_rule_parse (xmlDoc *doc, xmlNode *node)
{
  const char *name = (const char *) node->name;
  void *value;

  if (hash_find_entry (&classes, name, strlen (name), &value) == 0)
    {
      struct its_rule_ty *result;
      xmlNs **namespaces;

      result = its_rule_alloc ((struct its_rule_class_ty *) value, node);
      namespaces = xmlGetNsList (doc, node);
      if (namespaces)
        {
          size_t i;
          for (i = 0; namespaces[i] != NULL; i++)
            ;
          result->namespaces = XCALLOC (i + 1, xmlNs *);
          for (i = 0; namespaces[i] != NULL; i++)
            result->namespaces[i] = xmlCopyNamespace (namespaces[i]);
        }
      xmlFree (namespaces);
      return result;
    }

  return NULL;
}

static void
its_rule_destroy (struct its_rule_ty *pop)
{
  if (pop->methods->destructor)
    pop->methods->destructor (pop);
}

static void
init_classes (void)
{
#define ADD_RULE_CLASS(n, c) \
  hash_insert_entry (&classes, n, strlen (n), &c);

  ADD_RULE_CLASS ("translateRule", its_translate_rule_class);
  ADD_RULE_CLASS ("locNoteRule", its_localization_note_rule_class);
  ADD_RULE_CLASS ("withinTextRule", its_element_within_text_rule_class);
  ADD_RULE_CLASS ("preserveSpaceRule", its_preserve_space_rule_class);
  ADD_RULE_CLASS ("contextRule", its_extension_context_rule_class);
  ADD_RULE_CLASS ("escapeRule", its_extension_escape_rule_class);

#undef ADD_RULE_CLASS
}

struct its_rule_list_ty *
its_rule_list_alloc (void)
{
  struct its_rule_list_ty *result;

  if (classes.table == NULL)
    {
      hash_init (&classes, 10);
      init_classes ();
    }

  result = XCALLOC (1, struct its_rule_list_ty);
  return result;
}

void
its_rule_list_free (struct its_rule_list_ty *rules)
{
  size_t i;

  for (i = 0; i < rules->nitems; i++)
    {
      its_rule_destroy (rules->items[i]);
      free (rules->items[i]);
    }
  free (rules->items);
  its_pool_destroy (&rules->pool);
}

static bool
its_rule_list_add_from_doc (struct its_rule_list_ty *rules,
                            xmlDoc *doc)
{
  xmlNode *root, *node;

  root = xmlDocGetRootElement (doc);
  if (!(xmlStrEqual (root->name, BAD_CAST "rules")
        && xmlStrEqual (root->ns->href, BAD_CAST ITS_NS)))
    {
      error (0, 0, _("the root element is not \"rules\""
                     " under namespace %s"),
             ITS_NS);
      xmlFreeDoc (doc);
      return false;
    }

  for (node = root->children; node; node = node->next)
    {
      struct its_rule_ty *rule;

      rule = its_rule_parse (doc, node);
      if (rule != NULL)
        {
          if (rules->nitems == rules->nitems_max)
            {
              rules->nitems_max = 2 * rules->nitems_max + 1;
              rules->items =
                xrealloc (rules->items,
                          sizeof (struct its_rule_ty *) * rules->nitems_max);
            }
          rules->items[rules->nitems++] = rule;
        }
    }

  return true;
}

bool
its_rule_list_add_from_file (struct its_rule_list_ty *rules,
                             const char *filename)
{
  xmlDoc *doc;
  bool result;

  doc = xmlReadFile (filename, "utf-8",
                     XML_PARSE_NONET
                     | XML_PARSE_NOWARNING
                     | XML_PARSE_NOBLANKS
                     | XML_PARSE_NOERROR);
  if (doc == NULL)
    {
      xmlError *err = xmlGetLastError ();
      error (0, 0, _("cannot read %s: %s"), filename, err->message);
      return false;
    }

  result = its_rule_list_add_from_doc (rules, doc);
  xmlFreeDoc (doc);
  return result;
}

bool
its_rule_list_add_from_string (struct its_rule_list_ty *rules,
                               const char *rule)
{
  xmlDoc *doc;
  bool result;

  doc = xmlReadMemory (rule, strlen (rule),
                       "(internal)",
                       NULL,
                       XML_PARSE_NONET
                       | XML_PARSE_NOWARNING
                       | XML_PARSE_NOBLANKS
                       | XML_PARSE_NOERROR);
  if (doc == NULL)
    {
      xmlError *err = xmlGetLastError ();
      error (0, 0, _("cannot read %s: %s"), "(internal)", err->message);
      return false;
    }

  result = its_rule_list_add_from_doc (rules, doc);
  xmlFreeDoc (doc);
  return result;
}

static void
its_rule_list_apply (struct its_rule_list_ty *rules, xmlDoc *doc)
{
  size_t i;

  for (i = 0; i < rules->nitems; i++)
    {
      struct its_rule_ty *rule = rules->items[i];
      rule->methods->apply (rule, &rules->pool, doc);
    }
}

static struct its_value_list_ty *
its_rule_list_eval (its_rule_list_ty *rules, xmlNode *node)
{
  struct its_value_list_ty *result;
  size_t i;

  result = XCALLOC (1, struct its_value_list_ty);
  for (i = 0; i < rules->nitems; i++)
    {
      struct its_rule_ty *rule = rules->items[i];
      struct its_value_list_ty *values;

      values = rule->methods->eval (rule, &rules->pool, node);
      its_value_list_merge (result, values);
      its_value_list_destroy (values);
      free (values);
    }

  return result;
}

static bool
its_rule_list_is_translatable (its_rule_list_ty *rules,
                               xmlNode *node,
                               int depth)
{
  struct its_value_list_ty *values;
  const char *value;
  xmlNode *n;

  if (node->type != XML_ELEMENT_NODE
      && node->type != XML_ATTRIBUTE_NODE)
    return false;

  values = its_rule_list_eval (rules, node);

  /* Check if NODE has translate="yes".  */
  value = its_value_list_get_value (values, "translate");
  if (!(value && strcmp (value, "yes") == 0))
    {
      its_value_list_destroy (values);
      free (values);
      return false;
    }

  /* Check if NODE has withinText="yes", if NODE is not top-level.  */
  if (depth > 0)
    {
      value = its_value_list_get_value (values, "withinText");
      if (!(value && strcmp (value, "yes") == 0))
        {
          its_value_list_destroy (values);
          free (values);
          return false;
        }
    }

  its_value_list_destroy (values);
  free (values);

  for (n = node->children; n; n = n->next)
    {
      switch (n->type)
        {
        case XML_ELEMENT_NODE:
          if (!its_rule_list_is_translatable (rules, n, depth + 1))
            return false;
          break;

        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
        case XML_ENTITY_REF_NODE:
        case XML_COMMENT_NODE:
          break;

        default:
          return false;
        }
    }

  return true;
}

static void
its_rule_list_extract_nodes (its_rule_list_ty *rules,
                             struct its_node_list_ty *nodes,
                             xmlNode *node)
{
  if (node->type == XML_ELEMENT_NODE)
    {
      if (node->properties)
        {
          xmlAttr *attr;
          for (attr = node->properties; attr; attr = attr->next)
            {
              xmlNode *n = (xmlNode *) attr;
              if (its_rule_list_is_translatable (rules, n, 0))
                its_node_list_append (nodes, n);
            }
        }

      if (its_rule_list_is_translatable (rules, node, 0))
        its_node_list_append (nodes, node);
      else
        {
          xmlNode *n;
          for (n = node->children; n; n = n->next)
            its_rule_list_extract_nodes (rules, nodes, n);
        }
    }
}

static char *
_its_get_content (struct its_rule_list_ty *rules, xmlNode *node,
                  const char *pointer,
                  enum its_whitespace_type_ty whitespace,
                  bool no_escape)
{
  xmlXPathContext *context;
  xmlXPathObject *object;
  char *result = NULL;

  context = xmlXPathNewContext (node->doc);
  if (!context)
    {
      error (0, 0, _("cannot create XPath context"));
      return NULL;
    }

  {
    size_t i;

    for (i = 0; i < rules->nitems; i++)
      {
        struct its_rule_ty *rule = rules->items[i];
        if (rule->namespaces)
          {
            size_t j;
            for (j = 0; rule->namespaces[j] != NULL; j++)
              {
                xmlNs *ns = rule->namespaces[j];
                xmlXPathRegisterNs (context, ns->prefix, ns->href);
              }
          }
      }
  }

  xmlXPathSetContextNode (node, context);
  object = xmlXPathEvalExpression (BAD_CAST pointer, context);
  if (!object)
    {
      xmlXPathFreeContext (context);
      error (0, 0, _("cannot evaluate XPath location path: %s"),
             pointer);
      return NULL;
    }

  switch (object->type)
    {
    case XPATH_NODESET:
      {
        xmlNodeSet *nodes = object->nodesetval;
        string_list_ty sl;
        size_t i;

        string_list_init (&sl);
        for (i = 0; i < nodes->nodeNr; i++)
          {
            char *content = _its_collect_text_content (nodes->nodeTab[i],
                                                       whitespace,
                                                       no_escape);
            string_list_append (&sl, content);
            free (content);
          }
        result = string_list_concat (&sl);
        string_list_destroy (&sl);
      }
      break;

    case XPATH_STRING:
      result = xstrdup ((const char *) object->stringval);
      break;

    default:
      break;
    }

  xmlXPathFreeObject (object);
  xmlXPathFreeContext (context);

  return result;
}

static void
_its_comment_append (string_list_ty *comments, const char *data)
{
  /* Split multiline comment into lines, and remove leading and trailing
     whitespace.  */
  char *copy = xstrdup (data);
  char *p;
  char *q;

  for (p = copy; (q = strchr (p, '\n')) != NULL; p = q + 1)
    {
      while (p[0] == ' ' || p[0] == '\t')
        p++;
      while (q > p && (q[-1] == ' ' || q[-1] == '\t'))
        q--;
      *q = '\0';
      string_list_append (comments, p);
    }
  q = p + strlen (p);
  while (p[0] == ' ' || p[0] == '\t')
    p++;
  while (q > p && (q[-1] == ' ' || q[-1] == '\t'))
    q--;
  *q = '\0';
  string_list_append (comments, p);
  free (copy);
}

static void
its_rule_list_extract_text (its_rule_list_ty *rules,
                            xmlNode *node,
                            const char *logical_filename,
                            flag_context_list_table_ty *flag_table,
                            message_list_ty *mlp,
                            its_extract_callback_ty callback)
{
  if (node->type == XML_ELEMENT_NODE
      || node->type == XML_ATTRIBUTE_NODE)
    {
      struct its_value_list_ty *values;
      const char *value;
      char *msgid = NULL, *msgctxt = NULL, *comment = NULL;
      enum its_whitespace_type_ty whitespace;
      bool no_escape;
      
      values = its_rule_list_eval (rules, node);

      value = its_value_list_get_value (values, "locNote");
      if (value)
        comment = xstrdup (value);
      else
        {
          value = its_value_list_get_value (values, "escape");
          no_escape = value != NULL && strcmp (value, "no") == 0;

          value = its_value_list_get_value (values, "locNotePointer");
          if (value)
            comment = _its_get_content (rules, node, value, ITS_WHITESPACE_TRIM,
                                        no_escape);
        }

      if (comment != NULL && *comment != '\0')
        {
          string_list_ty comments;
          char *tmp;

          string_list_init (&comments);
          _its_comment_append (&comments, comment);
          tmp = string_list_join (&comments, "\n", '\0', false);
          free (comment);
          comment = tmp;
        }
      else
        /* Extract comments preceding the node.  */
        {
          xmlNode *sibling;
          string_list_ty comments;

          string_list_init (&comments);
          for (sibling = node->prev; sibling; sibling = sibling->prev)
            if (sibling->type != XML_COMMENT_NODE || sibling->prev == NULL)
              break;
          if (sibling)
            {
              if (sibling->type != XML_COMMENT_NODE)
                sibling = sibling->next;
              for (; sibling && sibling->type == XML_COMMENT_NODE;
                   sibling = sibling->next)
                {
                  xmlChar *content = xmlNodeGetContent (sibling);
                  _its_comment_append (&comments, (const char *) content);
                  xmlFree (content);
                }
              free (comment);
              comment = string_list_join (&comments, "\n", '\0', false);
              string_list_destroy (&comments);
            }
        }
      
      value = its_value_list_get_value (values, "space");
      if (value && strcmp (value, "preserve") == 0)
        whitespace = ITS_WHITESPACE_PRESERVE;
      else if (value && strcmp (value, "trim") == 0)
        whitespace = ITS_WHITESPACE_TRIM;
      else if (value && strcmp (value, "paragraph") == 0)
        whitespace = ITS_WHITESPACE_NORMALIZE_PARAGRAPH;
      else
        whitespace = ITS_WHITESPACE_NORMALIZE;

      value = its_value_list_get_value (values, "escape");
      no_escape = value != NULL && strcmp (value, "no") == 0;

      value = its_value_list_get_value (values, "contextPointer");
      if (value)
        msgctxt = _its_get_content (rules, node, value, ITS_WHITESPACE_PRESERVE,
                                    no_escape);

      value = its_value_list_get_value (values, "textPointer");
      if (value)
        msgid = _its_get_content (rules, node, value, ITS_WHITESPACE_PRESERVE,
                                  no_escape);
      its_value_list_destroy (values);
      free (values);

      if (msgid == NULL)
        msgid = _its_collect_text_content (node, whitespace, no_escape);
      if (*msgid != '\0')
        {
          lex_pos_ty pos;
          char *marker;

          pos.file_name = xstrdup (logical_filename);
          pos.line_number = xmlGetLineNo (node);

          if (node->type == XML_ELEMENT_NODE)
            {
              assert (node->parent);
              marker = xasprintf ("%s/%s", node->parent->name, node->name);
            }
          else
            {
              assert (node->parent && node->parent->parent);
              marker = xasprintf ("%s/%s@%s",
                                  node->parent->parent->name,
                                  node->parent->name,
                                  node->name);
            }

          if (msgctxt != NULL && *msgctxt == '\0')
            {
              free (msgctxt);
              msgctxt = NULL;
            }

          callback (mlp, msgctxt, msgid, &pos, comment, marker, whitespace);
          free (marker);
        }
      free (msgctxt);
      free (msgid);
      free (comment);
    }
}

void
its_rule_list_extract (its_rule_list_ty *rules,
                       FILE *fp, const char *real_filename,
                       const char *logical_filename,
                       flag_context_list_table_ty *flag_table,
                       msgdomain_list_ty *mdlp,
                       its_extract_callback_ty callback)
{
  xmlDoc *doc;
  struct its_node_list_ty nodes;
  size_t i;

  doc = xmlReadFd (fileno (fp), logical_filename, NULL,
                   XML_PARSE_NONET
                   | XML_PARSE_NOWARNING
                   | XML_PARSE_NOBLANKS
                   | XML_PARSE_NOERROR);
  if (doc == NULL)
    {
      xmlError *err = xmlGetLastError ();
      error (0, 0, _("cannot read %s: %s"), logical_filename, err->message);
      return;
    }

  its_rule_list_apply (rules, doc);

  memset (&nodes, 0, sizeof (struct its_node_list_ty));
  its_rule_list_extract_nodes (rules,
                               &nodes,
                               xmlDocGetRootElement (doc));

  for (i = 0; i < nodes.nitems; i++)
    its_rule_list_extract_text (rules, nodes.items[i],
                                logical_filename,
                                flag_table,
                                mdlp->item[0]->messages,
                                callback);

  free (nodes.items);
  xmlFreeDoc (doc);
}

struct its_merge_context_ty
{
  its_rule_list_ty *rules;
  xmlDoc *doc;
  struct its_node_list_ty nodes;
};

static void
its_merge_context_merge_node (struct its_merge_context_ty *context,
                              xmlNode *node,
                              const char *language,
                              message_list_ty *mlp)
{
  if (node->type == XML_ELEMENT_NODE)
    {
      struct its_value_list_ty *values;
      const char *value;
      char *msgid = NULL, *msgctxt = NULL;
      enum its_whitespace_type_ty whitespace;
      bool no_escape;

      values = its_rule_list_eval (context->rules, node);

      value = its_value_list_get_value (values, "space");
      if (value && strcmp (value, "preserve") == 0)
        whitespace = ITS_WHITESPACE_PRESERVE;
      else if (value && strcmp (value, "trim") == 0)
        whitespace = ITS_WHITESPACE_TRIM;
      else if (value && strcmp (value, "paragraph") == 0)
        whitespace = ITS_WHITESPACE_NORMALIZE_PARAGRAPH;
      else
        whitespace = ITS_WHITESPACE_NORMALIZE;

      value = its_value_list_get_value (values, "escape");
      no_escape = value != NULL && strcmp (value, "no") == 0;

      value = its_value_list_get_value (values, "contextPointer");
      if (value)
        msgctxt = _its_get_content (context->rules, node, value,
                                    ITS_WHITESPACE_PRESERVE, no_escape);

      value = its_value_list_get_value (values, "textPointer");
      if (value)
        msgid = _its_get_content (context->rules, node, value,
                                  ITS_WHITESPACE_PRESERVE, no_escape);
      its_value_list_destroy (values);
      free (values);

      if (msgid == NULL)
        msgid = _its_collect_text_content (node, whitespace, no_escape);
      if (*msgid != '\0')
        {
          message_ty *mp;

          mp = message_list_search (mlp, msgctxt, msgid);
          if (mp && *mp->msgstr != '\0')
            {
              xmlNode *translated;

              translated = xmlNewNode (node->ns, node->name);
              xmlSetProp (translated, BAD_CAST "xml:lang", BAD_CAST language);

              xmlNodeAddContent (translated, BAD_CAST mp->msgstr);
              xmlAddNextSibling (node, translated);
            }
        }
      free (msgctxt);
      free (msgid);
    }
}

void
its_merge_context_merge (its_merge_context_ty *context,
                         const char *language,
                         message_list_ty *mlp)
{
  size_t i;

  for (i = 0; i < context->nodes.nitems; i++)
    its_merge_context_merge_node (context, context->nodes.items[i],
                                  language,
                                  mlp);
}

struct its_merge_context_ty *
its_merge_context_alloc (its_rule_list_ty *rules,
                         const char *filename)
{
  xmlDoc *doc;
  struct its_merge_context_ty *result;

  doc = xmlReadFile (filename, NULL,
                     XML_PARSE_NONET
                     | XML_PARSE_NOWARNING
                     | XML_PARSE_NOBLANKS
                     | XML_PARSE_NOERROR);
  if (doc == NULL)
    {
      xmlError *err = xmlGetLastError ();
      error (0, 0, _("cannot read %s: %s"), filename, err->message);
      return NULL;
    }

  its_rule_list_apply (rules, doc);

  result = XMALLOC (struct its_merge_context_ty);
  result->rules = rules;
  result->doc = doc;

  /* Collect translatable nodes.  */
  memset (&result->nodes, 0, sizeof (struct its_node_list_ty));
  its_rule_list_extract_nodes (result->rules,
                               &result->nodes,
                               xmlDocGetRootElement (result->doc));

  return result;
}

void
its_merge_context_write (struct its_merge_context_ty *context,
                         FILE *fp)
{
  xmlDocFormatDump (fp, context->doc, 1);
}

void
its_merge_context_free (struct its_merge_context_ty *context)
{
  xmlFreeDoc (context->doc);
  free (context->nodes.items);
  free (context);
}
