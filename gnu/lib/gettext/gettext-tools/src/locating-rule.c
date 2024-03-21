/* XML resource locating rules
   Copyright (C) 2015, 2019-2020, 2023 Free Software Foundation, Inc.

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
# include <config.h>
#endif

/* Specification.  */
#include "locating-rule.h"

#include "basename-lgpl.h"
#include "concat-filename.h"
#include "c-strcase.h"

#if HAVE_DIRENT_H
# include <dirent.h>
#endif

#if HAVE_DIRENT_H
# define HAVE_DIR 1
#else
# define HAVE_DIR 0
#endif

#include "dir-list.h"
#include <errno.h>
#include "error.h"
#include "filename.h"
#include <fnmatch.h>
#include "gettext.h"
#include <libxml/parser.h>
#include <libxml/uri.h>
#include "xalloc.h"

#define _(str) gettext (str)

#define LOCATING_RULES_NS "https://www.gnu.org/s/gettext/ns/locating-rules/1.0"

struct document_locating_rule_ty
{
  char *ns;
  char *local_name;

  char *target;
};

struct document_locating_rule_list_ty
{
  struct document_locating_rule_ty *items;
  size_t nitems;
  size_t nitems_max;
};

struct locating_rule_ty
{
  char *pattern;
  char *name;

  struct document_locating_rule_list_ty doc_rules;
  char *target;
};

struct locating_rule_list_ty
{
  struct locating_rule_ty *items;
  size_t nitems;
  size_t nitems_max;
};

static char *
get_attribute (xmlNode *node, const char *attr)
{
  xmlChar *value;
  char *result;

  value = xmlGetProp (node, BAD_CAST attr);
  if (!value)
    {
      error (0, 0, _("cannot find attribute %s on %s"), attr, node->name);
      return NULL;
    }

  result = xstrdup ((const char *) value);
  xmlFree (value);

  return result;
}

static const char *
document_locating_rule_match (struct document_locating_rule_ty *rule,
                              xmlDoc *doc)
{
  xmlNode *root;

  root = xmlDocGetRootElement (doc);
  if (!root)
    {
      error (0, 0, _("cannot locate root element"));
      xmlFreeDoc (doc);
      return NULL;
    }

  if (rule->ns != NULL)
    {
      if (root->ns == NULL
          || !xmlStrEqual (root->ns->href, BAD_CAST rule->ns))
        return NULL;
    }

  if (rule->local_name != NULL)
    {
      if (!xmlStrEqual (root->name,
                        BAD_CAST rule->local_name))
        return NULL;
    }

  return rule->target;
}

static const char *
locating_rule_match (struct locating_rule_ty *rule,
                     const char *filename,
                     const char *name)
{
  if (name != NULL)
    {
      if (rule->name == NULL || c_strcasecmp (name, rule->name) != 0)
        return NULL;
    }
  else
    {
      const char *base;
      char *reduced;
      int err;

      base = strrchr (filename, '/');
      if (!base)
        base = filename;

      reduced = xstrdup (base);
      /* Remove a trailing ".in" - it's a generic suffix.  */
      while (strlen (reduced) >= 3
             && memcmp (reduced + strlen (reduced) - 3, ".in", 3) == 0)
        reduced[strlen (reduced) - 3] = '\0';

      err = fnmatch (rule->pattern, last_component (reduced), FNM_PATHNAME);
      free (reduced);
      if (err != 0)
        return NULL;
    }

  /* Check documentRules.  */
  if (rule->doc_rules.nitems > 0)
    {
      const char *target;
      xmlDoc *doc;
      size_t i;

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

      for (i = 0, target = NULL; i < rule->doc_rules.nitems; i++)
        {
          target =
            document_locating_rule_match (&rule->doc_rules.items[i], doc);
          if (target)
            break;
        }
      xmlFreeDoc (doc);
      if (target != NULL)
        return target;
    }

  if (rule->target != NULL)
    return rule->target;

  return NULL;
}

const char *
locating_rule_list_locate (struct locating_rule_list_ty *rules,
                           const char *filename,
                           const char *name)
{
  size_t i;

  for (i = 0; i < rules->nitems; i++)
    {
      if (IS_RELATIVE_FILE_NAME (filename))
        {
          int j;

          for (j = 0; ; ++j)
            {
              const char *dir = dir_list_nth (j);
              char *new_filename;
              const char *target;

              if (dir == NULL)
                break;
              
              new_filename = xconcatenated_filename (dir, filename, NULL);
              target = locating_rule_match (&rules->items[i], new_filename,
                                            name);
              free (new_filename);
              if (target != NULL)
                return target;
            }
        }
      else
        {
          const char *target =
            locating_rule_match (&rules->items[i], filename, name);

          if (target != NULL)
            return target;
        }
    }

  return NULL;
}

static void
missing_attribute (xmlNode *node, const char *attribute)
{
  error (0, 0, _("\"%s\" node does not have \"%s\""), node->name, attribute);
}

static void
document_locating_rule_destroy (struct document_locating_rule_ty *rule)
{
  free (rule->ns);
  free (rule->local_name);
  free (rule->target);
}

static void
document_locating_rule_list_add (struct document_locating_rule_list_ty *rules,
                                 xmlNode *node)
{
  struct document_locating_rule_ty rule;

  if (!xmlHasProp (node, BAD_CAST "target"))
    {
      missing_attribute (node, "target");
      return;
    }

  memset (&rule, 0, sizeof (struct document_locating_rule_ty));

  if (xmlHasProp (node, BAD_CAST "ns"))
    rule.ns = get_attribute (node, "ns");
  if (xmlHasProp (node, BAD_CAST "localName"))
    rule.local_name = get_attribute (node, "localName");
  rule.target = get_attribute (node, "target");

  if (rules->nitems == rules->nitems_max)
    {
      rules->nitems_max = 2 * rules->nitems_max + 1;
      rules->items =
        xrealloc (rules->items,
                  sizeof (struct document_locating_rule_ty)
                  * rules->nitems_max);
    }
  memcpy (&rules->items[rules->nitems++], &rule,
          sizeof (struct document_locating_rule_ty));
}

static void
locating_rule_destroy (struct locating_rule_ty *rule)
{
  size_t i;

  for (i = 0; i < rule->doc_rules.nitems; i++)
    document_locating_rule_destroy (&rule->doc_rules.items[i]);
  free (rule->doc_rules.items);

  free (rule->name);
  free (rule->pattern);
  free (rule->target);
}

static bool
locating_rule_list_add_from_file (struct locating_rule_list_ty *rules,
                                  const char *rule_file_name)
{
  xmlDoc *doc;
  xmlNode *root, *node;

  doc = xmlReadFile (rule_file_name, "utf-8",
                     XML_PARSE_NONET
                     | XML_PARSE_NOWARNING
                     | XML_PARSE_NOBLANKS
                     | XML_PARSE_NOERROR);
  if (doc == NULL)
    {
      error (0, 0, _("cannot read XML file %s"), rule_file_name);
      return false;
    }

  root = xmlDocGetRootElement (doc);
  if (!root)
    {
      error (0, 0, _("cannot locate root element"));
      xmlFreeDoc (doc);
      return false;
    }

  if (!(xmlStrEqual (root->name, BAD_CAST "locatingRules")
#if 0
        && root->ns
        && xmlStrEqual (root->ns->href, BAD_CAST LOCATING_RULES_NS)
#endif
        ))
    {
      error (0, 0, _("the root element is not \"locatingRules\""));
      xmlFreeDoc (doc);
      return false;
    }

  for (node = root->children; node; node = node->next)
    {
      if (xmlStrEqual (node->name, BAD_CAST "locatingRule"))
        {
          struct locating_rule_ty rule;

          if (!xmlHasProp (node, BAD_CAST "pattern"))
            {
              missing_attribute (node, "pattern");
              xmlFreeDoc (doc);
            }
          else
            {
              memset (&rule, 0, sizeof (struct locating_rule_ty));
              rule.pattern = get_attribute (node, "pattern");
              if (xmlHasProp (node, BAD_CAST "name"))
                rule.name = get_attribute (node, "name");
              if (xmlHasProp (node, BAD_CAST "target"))
                rule.target = get_attribute (node, "target");
              else
                {
                  xmlNode *n;

                  for (n = node->children; n; n = n->next)
                    {
                      if (xmlStrEqual (n->name, BAD_CAST "documentRule"))
                        document_locating_rule_list_add (&rule.doc_rules, n);
                    }
                }
              if (rules->nitems == rules->nitems_max)
                {
                  rules->nitems_max = 2 * rules->nitems_max + 1;
                  rules->items =
                    xrealloc (rules->items,
                              sizeof (struct locating_rule_ty) * rules->nitems_max);
                }
              memcpy (&rules->items[rules->nitems++], &rule,
                      sizeof (struct locating_rule_ty));
            }
        }
    }

  xmlFreeDoc (doc);
  return true;
}

bool
locating_rule_list_add_from_directory (struct locating_rule_list_ty *rules,
                                       const char *directory)
{
#if HAVE_DIR
  DIR *dirp;

  dirp = opendir (directory);
  if (dirp == NULL)
    return false;

  for (;;)
    {
      struct dirent *dp;

      errno = 0;
      dp = readdir (dirp);
      if (dp != NULL)
        {
          const char *name = dp->d_name;
          size_t namlen = strlen (name);

          if (namlen > 4 && memcmp (name + namlen - 4, ".loc", 4) == 0)
            {
              char *locator_file_name =
                xconcatenated_filename (directory, name, NULL);
              locating_rule_list_add_from_file (rules, locator_file_name);
              free (locator_file_name);
            }
        }
      else if (errno != 0)
        return false;
      else
        break;
    }
  if (closedir (dirp))
    return false;

#endif
  return true;
}

struct locating_rule_list_ty *
locating_rule_list_alloc (void)
{
  struct locating_rule_list_ty *result;

  xmlCheckVersion (LIBXML_VERSION);

  result = XCALLOC (1, struct locating_rule_list_ty);

  return result;
}

static void
locating_rule_list_destroy (struct locating_rule_list_ty *rules)
{
  while (rules->nitems-- > 0)
    locating_rule_destroy (&rules->items[rules->nitems]);
  free (rules->items);
}

void
locating_rule_list_free (struct locating_rule_list_ty *rules)
{
  if (rules != NULL)
    locating_rule_list_destroy (rules);
  free (rules);
}
