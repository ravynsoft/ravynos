/* prdbg.c -- Print out generic debugging information.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>.
   Tags style generation written by Salvador E. Tropea <set@computer.org>.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* This file prints out the generic debugging information, by
   supplying a set of routines to debug_write.  */

#include "sysdep.h"
#include <assert.h>
#include "bfd.h"
#include "libiberty.h"
#include "demangle.h"
#include "debug.h"
#include "budbg.h"

/* This is the structure we use as a handle for these routines.  */

struct pr_handle
{
  /* File to print information to.  */
  FILE *f;
  /* Current indentation level.  */
  unsigned int indent;
  /* Type stack.  */
  struct pr_stack *stack;
  /* Parameter number we are about to output.  */
  int parameter;
  /* The following are used only by the tags code (tg_).  */
  /* Name of the file we are using.  */
  char *filename;
  /* The BFD.  */
  bfd *abfd;
  /* The symbols table for this BFD.  */
  asymbol **syms;
  /* Pointer to a function to demangle symbols.  */
  char *(*demangler) (bfd *, const char *, int);
};

/* The type stack.  */

struct pr_stack
{
  /* Next element on the stack.  */
  struct pr_stack *next;
  /* This element.  */
  char *type;
  /* Current visibility of fields if this is a class.  */
  enum debug_visibility visibility;
  /* Name of the current method we are handling.  */
  char *method;
  /* The following are used only by the tags code (tg_).  */
  /* Type for the container (struct, union, class, union class).  */
  const char *flavor;
  /* A comma separated list of parent classes.  */
  char *parents;
};

static bool pr_start_compilation_unit (void *, const char *);
static bool pr_start_source (void *, const char *);
static bool pr_empty_type (void *);
static bool pr_void_type (void *);
static bool pr_int_type (void *, unsigned int, bool);
static bool pr_float_type (void *, unsigned int);
static bool pr_complex_type (void *, unsigned int);
static bool pr_bool_type (void *, unsigned int);
static bool pr_enum_type
  (void *, const char *, const char **, bfd_signed_vma *);
static bool pr_pointer_type (void *);
static bool pr_function_type (void *, int, bool);
static bool pr_reference_type (void *);
static bool pr_range_type (void *, bfd_signed_vma, bfd_signed_vma);
static bool pr_array_type (void *, bfd_signed_vma, bfd_signed_vma, bool);
static bool pr_set_type (void *, bool);
static bool pr_offset_type (void *);
static bool pr_method_type (void *, bool, int, bool);
static bool pr_const_type (void *);
static bool pr_volatile_type (void *);
static bool pr_start_struct_type
  (void *, const char *, unsigned int, bool, unsigned int);
static bool pr_struct_field
  (void *, const char *, bfd_vma, bfd_vma, enum debug_visibility);
static bool pr_end_struct_type (void *);
static bool pr_start_class_type
  (void *, const char *, unsigned int, bool, unsigned int, bool, bool);
static bool pr_class_static_member
  (void *, const char *, const char *, enum debug_visibility);
static bool pr_class_baseclass
  (void *, bfd_vma, bool, enum debug_visibility);
static bool pr_class_start_method (void *, const char *);
static bool pr_class_method_variant
  (void *, const char *, enum debug_visibility, bool, bool, bfd_vma, bool);
static bool pr_class_static_method_variant
  (void *, const char *, enum debug_visibility, bool, bool);
static bool pr_class_end_method (void *);
static bool pr_end_class_type (void *);
static bool pr_typedef_type (void *, const char *);
static bool pr_tag_type
  (void *, const char *, unsigned int, enum debug_type_kind);
static bool pr_typdef (void *, const char *);
static bool pr_tag (void *, const char *);
static bool pr_int_constant (void *, const char *, bfd_vma);
static bool pr_float_constant (void *, const char *, double);
static bool pr_typed_constant (void *, const char *, bfd_vma);
static bool pr_variable (void *, const char *, enum debug_var_kind, bfd_vma);
static bool pr_start_function (void *, const char *, bool);
static bool pr_function_parameter
  (void *, const char *, enum debug_parm_kind, bfd_vma);
static bool pr_start_block (void *, bfd_vma);
static bool pr_end_block (void *, bfd_vma);
static bool pr_end_function (void *);
static bool pr_lineno (void *, const char *, unsigned long, bfd_vma);

static const char *visibility_name (enum debug_visibility);

/* Tags style replacements.  */
static bool tg_start_compilation_unit (void *, const char *);
static bool tg_start_source (void *, const char *);
static bool tg_enum_type
  (void *, const char *, const char **, bfd_signed_vma *);
static bool tg_start_struct_type
  (void *, const char *, unsigned int, bool, unsigned int);
static bool pr_struct_field
  (void *, const char *, bfd_vma, bfd_vma, enum debug_visibility);
static bool tg_struct_field
  (void *, const char *, bfd_vma, bfd_vma, enum debug_visibility);
static bool tg_struct_field
  (void *, const char *, bfd_vma, bfd_vma, enum debug_visibility);
static bool tg_end_struct_type (void *);
static bool tg_start_class_type
  (void *, const char *, unsigned int, bool, unsigned int, bool, bool);
static bool tg_class_static_member
  (void *, const char *, const char *, enum debug_visibility);
static bool tg_class_baseclass (void *, bfd_vma, bool, enum debug_visibility);
static bool tg_class_method_variant
  (void *, const char *, enum debug_visibility, bool, bool, bfd_vma, bool);
static bool tg_class_static_method_variant
  (void *, const char *, enum debug_visibility, bool, bool);
static bool tg_end_class_type (void *);
static bool tg_tag_type
  (void *, const char *, unsigned int, enum debug_type_kind);
static bool tg_typdef (void *, const char *);
static bool tg_tag (void *, const char *);
static bool tg_int_constant (void *, const char *, bfd_vma);
static bool tg_float_constant (void *, const char *, double);
static bool tg_typed_constant (void *, const char *, bfd_vma);
static bool tg_variable (void *, const char *, enum debug_var_kind, bfd_vma);
static bool tg_start_function (void *, const char *, bool);
static bool tg_function_parameter
  (void *, const char *, enum debug_parm_kind, bfd_vma);
static bool tg_start_block (void *, bfd_vma);
static bool tg_end_block (void *, bfd_vma);
static bool tg_lineno (void *, const char *, unsigned long, bfd_vma);

static const struct debug_write_fns pr_fns =
{
  pr_start_compilation_unit,
  pr_start_source,
  pr_empty_type,
  pr_void_type,
  pr_int_type,
  pr_float_type,
  pr_complex_type,
  pr_bool_type,
  pr_enum_type,
  pr_pointer_type,
  pr_function_type,
  pr_reference_type,
  pr_range_type,
  pr_array_type,
  pr_set_type,
  pr_offset_type,
  pr_method_type,
  pr_const_type,
  pr_volatile_type,
  pr_start_struct_type,
  pr_struct_field,
  pr_end_struct_type,
  pr_start_class_type,
  pr_class_static_member,
  pr_class_baseclass,
  pr_class_start_method,
  pr_class_method_variant,
  pr_class_static_method_variant,
  pr_class_end_method,
  pr_end_class_type,
  pr_typedef_type,
  pr_tag_type,
  pr_typdef,
  pr_tag,
  pr_int_constant,
  pr_float_constant,
  pr_typed_constant,
  pr_variable,
  pr_start_function,
  pr_function_parameter,
  pr_start_block,
  pr_end_block,
  pr_end_function,
  pr_lineno
};

static const struct debug_write_fns tg_fns =
{
  tg_start_compilation_unit,
  tg_start_source,
  pr_empty_type,		/* Same, push_type.  */
  pr_void_type,			/* Same, push_type.  */
  pr_int_type,			/* Same, push_type.  */
  pr_float_type,		/* Same, push_type.  */
  pr_complex_type,		/* Same, push_type.  */
  pr_bool_type,			/* Same, push_type.  */
  tg_enum_type,
  pr_pointer_type,		/* Same, changes to pointer.  */
  pr_function_type,		/* Same, push_type.  */
  pr_reference_type,		/* Same, changes to reference.  */
  pr_range_type,		/* FIXME: What's that?.  */
  pr_array_type,		/* Same, push_type.  */
  pr_set_type,			/* FIXME: What's that?.  */
  pr_offset_type,		/* FIXME: What's that?.  */
  pr_method_type,		/* Same.  */
  pr_const_type,		/* Same, changes to const.  */
  pr_volatile_type,		/* Same, changes to volatile.  */
  tg_start_struct_type,
  tg_struct_field,
  tg_end_struct_type,
  tg_start_class_type,
  tg_class_static_member,
  tg_class_baseclass,
  pr_class_start_method,	/* Same, remembers that's a method.  */
  tg_class_method_variant,
  tg_class_static_method_variant,
  pr_class_end_method,		/* Same, forgets that's a method.  */
  tg_end_class_type,
  pr_typedef_type,		/* Same, just push type.  */
  tg_tag_type,
  tg_typdef,
  tg_tag,
  tg_int_constant,		/* Untested.  */
  tg_float_constant,		/* Untested.  */
  tg_typed_constant,		/* Untested.  */
  tg_variable,
  tg_start_function,
  tg_function_parameter,
  tg_start_block,
  tg_end_block,
  pr_end_function,		/* Same, does nothing.  */
  tg_lineno
};

static int demangle_flags = DMGL_ANSI | DMGL_PARAMS;

/* Print out the generic debugging information recorded in dhandle.  */

bool
print_debugging_info (FILE *f, void *dhandle, bfd *abfd, asymbol **syms,
		      char * (*demangler) (struct bfd *, const char *, int),
		      bool as_tags)
{
  struct pr_handle info;

  info.f = f;
  info.indent = 0;
  info.stack = NULL;
  info.parameter = 0;
  info.filename = NULL;
  info.abfd = abfd;
  info.syms = syms;
  info.demangler = demangler;

  if (as_tags)
    {
      fputs ("!_TAG_FILE_FORMAT\t2\t/extended format/\n", f);
      fputs ("!_TAG_FILE_SORTED\t0\t/0=unsorted, 1=sorted/\n", f);
      fputs ("!_TAG_PROGRAM_AUTHOR\tIan Lance Taylor, Salvador E. Tropea and others\t//\n", f);
      fputs ("!_TAG_PROGRAM_NAME\tobjdump\t/From GNU binutils/\n", f);
    }

  bool ret = debug_write (dhandle, as_tags ? &tg_fns : &pr_fns, &info);
  while (info.stack != NULL)
    {
      struct pr_stack *s = info.stack;
      info.stack = s->next;
      free (s->type);
      free (s->method);
      free (s->parents);
      free (s);
    }
  free (info.filename);
  return ret;
}

/* Indent to the current indentation level.  */

static void
indent (struct pr_handle *info)
{
  unsigned int i;

  for (i = 0; i < info->indent; i++)
    putc (' ', info->f);
}

/* Push a type on the type stack.  */

static bool
push_type (struct pr_handle *info, const char *type)
{
  struct pr_stack *n;

  if (type == NULL)
    return false;

  n = xmalloc (sizeof *n);
  memset (n, 0, sizeof *n);

  n->type = xstrdup (type);
  n->visibility = DEBUG_VISIBILITY_IGNORE;
  n->method = NULL;
  n->next = info->stack;
  info->stack = n;

  return true;
}

/* Prepend a string onto the type on the top of the type stack.  */

static bool
prepend_type (struct pr_handle *info, const char *s)
{
  char *n;

  assert (info->stack != NULL);

  n = xmalloc (strlen (s) + strlen (info->stack->type) + 1);
  sprintf (n, "%s%s", s, info->stack->type);
  free (info->stack->type);
  info->stack->type = n;

  return true;
}

/* Append a string to the type on the top of the type stack.  */

static bool
append_type (struct pr_handle *info, const char *s)
{
  unsigned int len;

  if (s == NULL)
    return false;

  assert (info->stack != NULL);

  len = strlen (info->stack->type);
  info->stack->type = xrealloc (info->stack->type, len + strlen (s) + 1);
  strcpy (info->stack->type + len, s);

  return true;
}

/* Append a string to the parents on the top of the type stack.  */

static bool
append_parent (struct pr_handle *info, const char *s)
{
  unsigned int len;

  if (s == NULL)
    return false;

  assert (info->stack != NULL);

  len = info->stack->parents ? strlen (info->stack->parents) : 0;
  info->stack->parents = xrealloc (info->stack->parents, len + strlen (s) + 1);
  strcpy (info->stack->parents + len, s);

  return true;
}

/* We use an underscore to indicate where the name should go in a type
   string.  This function substitutes a string for the underscore.  If
   there is no underscore, the name follows the type.  */

static bool
substitute_type (struct pr_handle *info, const char *s)
{
  char *u;

  assert (info->stack != NULL);

  u = strchr (info->stack->type, '|');
  if (u != NULL)
    {
      char *n;

      n = xmalloc (strlen (info->stack->type) + strlen (s));

      memcpy (n, info->stack->type, u - info->stack->type);
      strcpy (n + (u - info->stack->type), s);
      strcat (n, u + 1);

      free (info->stack->type);
      info->stack->type = n;

      return true;
    }

  if (strchr (s, '|') != NULL
      && (strchr (info->stack->type, '{') != NULL
	  || strchr (info->stack->type, '(') != NULL))
    {
      if (! prepend_type (info, "(")
	  || ! append_type (info, ")"))
	return false;
    }

  if (*s == '\0')
    return true;

  return (append_type (info, " ")
	  && append_type (info, s));
}

/* Indent the type at the top of the stack by appending spaces.  */

static bool
indent_type (struct pr_handle *info)
{
  unsigned int i;

  for (i = 0; i < info->indent; i++)
    {
      if (! append_type (info, " "))
	return false;
    }

  return true;
}

/* Pop a type from the type stack.  */

static char *
pop_type (struct pr_handle *info)
{
  struct pr_stack *o;
  char *ret;

  assert (info->stack != NULL);

  o = info->stack;
  info->stack = o->next;
  ret = o->type;
  free (o);

  return ret;
}

/* Print a VMA value into a string.  */

static void
print_vma (bfd_vma vma, char *buf, bool unsignedp, bool hexp)
{
  if (hexp)
    sprintf (buf, "%#" PRIx64, (uint64_t) vma);
  else if (unsignedp)
    sprintf (buf, "%" PRIu64, (uint64_t) vma);
  else
    sprintf (buf, "%" PRId64, (int64_t) vma);
}

/* Start a new compilation unit.  */

static bool
pr_start_compilation_unit (void *p, const char *filename)
{
  struct pr_handle *info = (struct pr_handle *) p;

  assert (info->indent == 0);

  fprintf (info->f, "%s:\n", filename);

  return true;
}

/* Start a source file within a compilation unit.  */

static bool
pr_start_source (void *p, const char *filename)
{
  struct pr_handle *info = (struct pr_handle *) p;

  assert (info->indent == 0);

  fprintf (info->f, " %s:\n", filename);

  return true;
}

/* Push an empty type onto the type stack.  */

static bool
pr_empty_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  return push_type (info, "<undefined>");
}

/* Push a void type onto the type stack.  */

static bool
pr_void_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  return push_type (info, "void");
}

/* Push an integer type onto the type stack.  */

static bool
pr_int_type (void *p, unsigned int size, bool unsignedp)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[40];

  sprintf (ab, "%sint%d", unsignedp ? "u" : "", size * 8);
  return push_type (info, ab);
}

/* Push a floating type onto the type stack.  */

static bool
pr_float_type (void *p, unsigned int size)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[40];

  if (size == 4)
    return push_type (info, "float");
  else if (size == 8)
    return push_type (info, "double");

  sprintf (ab, "float%d", size * 8);
  return push_type (info, ab);
}

/* Push a complex type onto the type stack.  */

static bool
pr_complex_type (void *p, unsigned int size)
{
  struct pr_handle *info = (struct pr_handle *) p;

  if (! pr_float_type (p, size))
    return false;

  return prepend_type (info, "complex ");
}

/* Push a bool type onto the type stack.  */

static bool
pr_bool_type (void *p, unsigned int size)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[40];

  sprintf (ab, "bool%d", size * 8);

  return push_type (info, ab);
}

/* Push an enum type onto the type stack.  */

static bool
pr_enum_type (void *p, const char *tag, const char **names,
	      bfd_signed_vma *values)
{
  struct pr_handle *info = (struct pr_handle *) p;
  unsigned int i;
  bfd_signed_vma val;

  if (! push_type (info, "enum "))
    return false;
  if (tag != NULL)
    {
      if (! append_type (info, tag)
	  || ! append_type (info, " "))
	return false;
    }
  if (! append_type (info, "{ "))
    return false;

  if (names == NULL)
    {
      if (! append_type (info, "/* undefined */"))
	return false;
    }
  else
    {
      val = 0;
      for (i = 0; names[i] != NULL; i++)
	{
	  if (i > 0)
	    {
	      if (! append_type (info, ", "))
		return false;
	    }

	  if (! append_type (info, names[i]))
	    return false;

	  if (values[i] != val)
	    {
	      char ab[22];

	      print_vma (values[i], ab, false, false);
	      if (! append_type (info, " = ")
		  || ! append_type (info, ab))
		return false;
	      val = values[i];
	    }

	  ++val;
	}
    }

  return append_type (info, " }");
}

/* Turn the top type on the stack into a pointer.  */

static bool
pr_pointer_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *s;

  assert (info->stack != NULL);

  s = strchr (info->stack->type, '|');
  if (s != NULL && s[1] == '[')
    return substitute_type (info, "(*|)");
  return substitute_type (info, "*|");
}

/* Turn the top type on the stack into a function returning that type.  */

static bool
pr_function_type (void *p, int argcount, bool varargs)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char **arg_types;
  unsigned int len;
  char *s;

  assert (info->stack != NULL);

  len = 10;

  if (argcount <= 0)
    {
      arg_types = NULL;
      len += 15;
    }
  else
    {
      int i;

      arg_types = xmalloc (argcount * sizeof (*arg_types));
      for (i = argcount - 1; i >= 0; i--)
	{
	  if (!substitute_type (info, "")
	      || (arg_types[i] = pop_type (info)) == NULL)
	    {
	      for (int j = i + 1; j < argcount; j++)
		free (arg_types[j]);
	      free (arg_types);
	      return false;
	    }
	  len += strlen (arg_types[i]) + 2;
	}
      if (varargs)
	len += 5;
    }

  /* Now the return type is on the top of the stack.  */

  s = xmalloc (len);
  strcpy (s, "(|) (");

  if (argcount < 0)
    strcat (s, "/* unknown */");
  else
    {
      int i;

      for (i = 0; i < argcount; i++)
	{
	  if (i > 0)
	    strcat (s, ", ");
	  strcat (s, arg_types[i]);
	  free (arg_types[i]);
	}
      if (varargs)
	{
	  if (i > 0)
	    strcat (s, ", ");
	  strcat (s, "...");
	}
      free (arg_types);
    }

  strcat (s, ")");

  bool ret = substitute_type (info, s);
  free (s);
  return ret;
}

/* Turn the top type on the stack into a reference to that type.  */

static bool
pr_reference_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  assert (info->stack != NULL);

  return substitute_type (info, "&|");
}

/* Make a range type.  */

static bool
pr_range_type (void *p, bfd_signed_vma lower, bfd_signed_vma upper)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char abl[22], abu[22];

  assert (info->stack != NULL);

  if (! substitute_type (info, ""))
    return false;

  print_vma (lower, abl, false, false);
  print_vma (upper, abu, false, false);

  return (prepend_type (info, "range (")
	  && append_type (info, "):")
	  && append_type (info, abl)
	  && append_type (info, ":")
	  && append_type (info, abu));
}

/* Make an array type.  */

static bool
pr_array_type (void *p, bfd_signed_vma lower, bfd_signed_vma upper,
	       bool stringp)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *range_type;
  char abl[22], abu[22], ab[50];

  range_type = pop_type (info);
  if (range_type == NULL)
    return false;

  if (lower == 0)
    {
      if (upper == -1)
	sprintf (ab, "|[]");
      else
	{
	  print_vma (upper + 1, abu, false, false);
	  sprintf (ab, "|[%s]", abu);
	}
    }
  else
    {
      print_vma (lower, abl, false, false);
      print_vma (upper, abu, false, false);
      sprintf (ab, "|[%s:%s]", abl, abu);
    }

  if (! substitute_type (info, ab))
    goto fail;

  if (strcmp (range_type, "int") != 0)
    {
      if (! append_type (info, ":")
	  || ! append_type (info, range_type))
	goto fail;
    }

  if (stringp)
    {
      if (! append_type (info, " /* string */"))
	goto fail;
    }

  free (range_type);
  return true;

 fail:
  free (range_type);
  return false;
}

/* Make a set type.  */

static bool
pr_set_type (void *p, bool bitstringp)
{
  struct pr_handle *info = (struct pr_handle *) p;

  if (! substitute_type (info, ""))
    return false;

  if (! prepend_type (info, "set { ")
      || ! append_type (info, " }"))
    return false;

  if (bitstringp)
    {
      if (! append_type (info, "/* bitstring */"))
	return false;
    }

  return true;
}

/* Make an offset type.  */

static bool
pr_offset_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  if (! substitute_type (info, ""))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  bool ret = (substitute_type (info, "")
	      && prepend_type (info, " ")
	      && prepend_type (info, t)
	      && append_type (info, "::|"));
  free (t);
  return ret;
}

/* Make a method type.  */

static bool
pr_method_type (void *p, bool domain, int argcount, bool varargs)
{
  struct pr_handle *info = (struct pr_handle *) p;
  unsigned int len;
  char *domain_type = NULL, *free_domain = NULL;
  char **arg_types;
  char *s;

  len = 10;

  if (domain)
    {
      if (! substitute_type (info, ""))
	return false;
      domain_type = pop_type (info);
      if (domain_type == NULL)
	return false;
      free_domain = domain_type;
      if (startswith (domain_type, "class ")
	  && strchr (domain_type + sizeof "class " - 1, ' ') == NULL)
	domain_type += sizeof "class " - 1;
      else if (startswith (domain_type, "union class ")
	       && (strchr (domain_type + sizeof "union class " - 1, ' ')
		   == NULL))
	domain_type += sizeof "union class " - 1;
      len += strlen (domain_type);
    }

  if (argcount <= 0)
    {
      arg_types = NULL;
      len += 15;
    }
  else
    {
      int i;

      arg_types = xmalloc (argcount * sizeof (*arg_types));
      for (i = argcount - 1; i >= 0; i--)
	{
	  if (!substitute_type (info, "")
	      || (arg_types[i] = pop_type (info)) == NULL)
	    {
	      for (int j = i + 1; j < argcount; ++j)
		free (arg_types[j]);
	      free (arg_types);
	      return false;
	    }
	  len += strlen (arg_types[i]) + 2;
	}
      if (varargs)
	len += 5;
    }

  /* Now the return type is on the top of the stack.  */

  s = xmalloc (len);
  *s = 0;
  if (domain)
    {
      strcpy (s, domain_type);
      free (free_domain);
    }
  strcat (s, "::| (");

  if (argcount < 0)
    strcat (s, "/* unknown */");
  else
    {
      int i;

      for (i = 0; i < argcount; i++)
	{
	  if (i > 0)
	    strcat (s, ", ");
	  strcat (s, arg_types[i]);
	  free (arg_types[i]);
	}
      if (varargs)
	{
	  if (i > 0)
	    strcat (s, ", ");
	  strcat (s, "...");
	}
      free (arg_types);
    }

  strcat (s, ")");

  bool ret = substitute_type (info, s);
  free (s);
  return ret;
}

/* Make a const qualified type.  */

static bool
pr_const_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  return substitute_type (info, "const |");
}

/* Make a volatile qualified type.  */

static bool
pr_volatile_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  return substitute_type (info, "volatile |");
}

/* Start accumulating a struct type.  */

static bool
pr_start_struct_type (void *p, const char *tag, unsigned int id,
		      bool structp, unsigned int size)
{
  struct pr_handle *info = (struct pr_handle *) p;

  info->indent += 2;

  if (! push_type (info, structp ? "struct " : "union "))
    return false;
  if (tag != NULL)
    {
      if (! append_type (info, tag))
	return false;
    }
  else
    {
      char idbuf[20];

      sprintf (idbuf, "%%anon%u", id);
      if (! append_type (info, idbuf))
	return false;
    }

  if (! append_type (info, " {"))
    return false;
  if (size != 0 || tag != NULL)
    {
      char ab[30];

      if (! append_type (info, " /*"))
	return false;

      if (size != 0)
	{
	  sprintf (ab, " size %u", size);
	  if (! append_type (info, ab))
	    return false;
	}
      if (tag != NULL)
	{
	  sprintf (ab, " id %u", id);
	  if (! append_type (info, ab))
	    return false;
	}
      if (! append_type (info, " */"))
	return false;
    }
  if (! append_type (info, "\n"))
    return false;

  info->stack->visibility = DEBUG_VISIBILITY_PUBLIC;

  return indent_type (info);
}

/* Output the visibility of a field in a struct.  */

static bool
pr_fix_visibility (struct pr_handle *info, enum debug_visibility visibility)
{
  const char *s = NULL;
  char *t;
  unsigned int len;

  assert (info->stack != NULL);

  if (info->stack->visibility == visibility)
    return true;

  switch (visibility)
    {
    case DEBUG_VISIBILITY_PUBLIC:
      s = "public";
      break;
    case DEBUG_VISIBILITY_PRIVATE:
      s = "private";
      break;
    case DEBUG_VISIBILITY_PROTECTED:
      s = "protected";
      break;
    case DEBUG_VISIBILITY_IGNORE:
      s = "/* ignore */";
      break;
    default:
      abort ();
      return false;
    }

  /* Trim off a trailing space in the struct string, to make the
     output look a bit better, then stick on the visibility string.  */

  t = info->stack->type;
  len = strlen (t);
  assert (t[len - 1] == ' ');
  t[len - 1] = '\0';

  if (! append_type (info, s)
      || ! append_type (info, ":\n")
      || ! indent_type (info))
    return false;

  info->stack->visibility = visibility;

  return true;
}

/* Add a field to a struct type.  */

static bool
pr_struct_field (void *p, const char *name, bfd_vma bitpos, bfd_vma bitsize,
		 enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];
  char *t;

  if (! substitute_type (info, name))
    return false;

  if (! append_type (info, "; /* "))
    return false;

  if (bitsize != 0)
    {
      print_vma (bitsize, ab, true, false);
      if (! append_type (info, "bitsize ")
	  || ! append_type (info, ab)
	  || ! append_type (info, ", "))
	return false;
    }

  print_vma (bitpos, ab, true, false);
  if (! append_type (info, "bitpos ")
      || ! append_type (info, ab)
      || ! append_type (info, " */\n")
      || ! indent_type (info))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  bool ret = pr_fix_visibility (info, visibility) && append_type (info, t);
  free (t);
  return ret;
}

/* Finish a struct type.  */

static bool
pr_end_struct_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *s;

  assert (info->stack != NULL);
  assert (info->indent >= 2);

  info->indent -= 2;

  /* Change the trailing indentation to have a close brace.  */
  s = info->stack->type + strlen (info->stack->type) - 2;
  assert (s[0] == ' ' && s[1] == ' ' && s[2] == '\0');

  *s++ = '}';
  *s = '\0';

  return true;
}

/* Start a class type.  */

static bool
pr_start_class_type (void *p, const char *tag, unsigned int id,
		     bool structp, unsigned int size,
		     bool vptr, bool ownvptr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *tv = NULL;
  bool ret = false;

  info->indent += 2;

  if (vptr && ! ownvptr)
    {
      tv = pop_type (info);
      if (tv == NULL)
	return false;
    }

  if (! push_type (info, structp ? "class " : "union class "))
    goto out;
  if (tag != NULL)
    {
      if (! append_type (info, tag))
	goto out;
    }
  else
    {
      char idbuf[20];

      sprintf (idbuf, "%%anon%u", id);
      if (! append_type (info, idbuf))
	goto out;
    }

  if (! append_type (info, " {"))
    goto out;
  if (size != 0 || vptr || ownvptr || tag != NULL)
    {
      if (! append_type (info, " /*"))
	goto out;

      if (size != 0)
	{
	  char ab[20];

	  sprintf (ab, "%u", size);
	  if (! append_type (info, " size ")
	      || ! append_type (info, ab))
	    goto out;
	}

      if (vptr)
	{
	  if (! append_type (info, " vtable "))
	    goto out;
	  if (ownvptr)
	    {
	      if (! append_type (info, "self "))
		goto out;
	    }
	  else
	    {
	      if (! append_type (info, tv)
		  || ! append_type (info, " "))
		goto out;
	    }
	}

      if (tag != NULL)
	{
	  char ab[30];

	  sprintf (ab, " id %u", id);
	  if (! append_type (info, ab))
	    goto out;
	}

      if (! append_type (info, " */"))
	goto out;
    }

  info->stack->visibility = DEBUG_VISIBILITY_PRIVATE;

  ret = append_type (info, "\n") && indent_type (info);
 out:
  free (tv);
  return ret;
}

/* Add a static member to a class.  */

static bool
pr_class_static_member (void *p, const char *name, const char *physname,
			enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  if (! substitute_type (info, name))
    return false;

  if (! prepend_type (info, "static ")
      || ! append_type (info, "; /* ")
      || ! append_type (info, physname)
      || ! append_type (info, " */\n")
      || ! indent_type (info))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  bool ret = pr_fix_visibility (info, visibility) && append_type (info, t);
  free (t);
  return ret;
}

/* Add a base class to a class.  */

static bool
pr_class_baseclass (void *p, bfd_vma bitpos, bool is_virtual,
		    enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  const char *prefix;
  char ab[22];
  char *s, *l, *n;

  assert (info->stack != NULL && info->stack->next != NULL);

  if (! substitute_type (info, ""))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  /* Push it back on to take advantage of the prepend_type and
     append_type routines.  */
  if (! push_type (info, t + (startswith (t, "class ")
			      ? sizeof "class " - 1 : 0)))
    {
      free (t);
      return false;
    }
  free (t);

  if (is_virtual)
    {
      if (! prepend_type (info, "virtual "))
	return false;
    }

  switch (visibility)
    {
    case DEBUG_VISIBILITY_PUBLIC:
      prefix = "public ";
      break;
    case DEBUG_VISIBILITY_PROTECTED:
      prefix = "protected ";
      break;
    case DEBUG_VISIBILITY_PRIVATE:
      prefix = "private ";
      break;
    default:
      prefix = "/* unknown visibility */ ";
      break;
    }

  if (! prepend_type (info, prefix))
    return false;

  if (bitpos != 0)
    {
      print_vma (bitpos, ab, true, false);
      if (! append_type (info, " /* bitpos ")
	  || ! append_type (info, ab)
	  || ! append_type (info, " */"))
	return false;
    }

  /* Now the top of the stack is something like "public A / * bitpos
     10 * /".  The next element on the stack is something like "class
     xx { / * size 8 * /\n...".  We want to substitute the top of the
     stack in before the {.  */
  s = strchr (info->stack->next->type, '{');
  assert (s != NULL);
  --s;

  /* If there is already a ':', then we already have a baseclass, and
     we must append this one after a comma.  */
  for (l = info->stack->next->type; l != s; l++)
    if (*l == ':')
      break;
  if (! prepend_type (info, l == s ? " : " : ", "))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  n = xmalloc (strlen (info->stack->type) + strlen (t) + 1);
  memcpy (n, info->stack->type, s - info->stack->type);
  strcpy (n + (s - info->stack->type), t);
  strcat (n, s);

  free (info->stack->type);
  info->stack->type = n;

  free (t);

  return true;
}

/* Start adding a method to a class.  */

static bool
pr_class_start_method (void *p, const char *name)
{
  struct pr_handle *info = (struct pr_handle *) p;

  assert (info->stack != NULL);
  free (info->stack->method);
  info->stack->method = xstrdup (name);
  return true;
}

/* Add a variant to a method.  */

static bool
pr_class_method_variant (void *p, const char *physname,
			 enum debug_visibility visibility,
			 bool constp, bool volatilep,
			 bfd_vma voffset, bool context)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *method_type;
  char *context_type;
  bool ret = false;

  assert (info->stack != NULL);
  assert (info->stack->next != NULL);

  /* Put the const and volatile qualifiers on the type.  */
  if (volatilep)
    {
      if (! append_type (info, " volatile"))
	return false;
    }
  if (constp)
    {
      if (! append_type (info, " const"))
	return false;
    }

  /* Stick the name of the method into its type.  */
  if (! substitute_type (info,
			 (context
			  ? info->stack->next->next->method
			  : info->stack->next->method)))
    return false;

  /* Get the type.  */
  method_type = pop_type (info);
  if (method_type == NULL)
    return false;

  /* Pull off the context type if there is one.  */
  if (! context)
    context_type = NULL;
  else
    {
      context_type = pop_type (info);
      if (context_type == NULL)
	goto out;
    }

  /* Now the top of the stack is the class.  */

  if (! pr_fix_visibility (info, visibility))
    goto out;

  if (! append_type (info, method_type)
      || ! append_type (info, " /* ")
      || ! append_type (info, physname)
      || ! append_type (info, " "))
    goto out;
  if (context || voffset != 0)
    {
      char ab[22];

      if (context)
	{
	  if (! append_type (info, "context ")
	      || ! append_type (info, context_type)
	      || ! append_type (info, " "))
	    goto out;
	}
      print_vma (voffset, ab, true, false);
      if (! append_type (info, "voffset ")
	  || ! append_type (info, ab))
	goto out;
    }

  ret = append_type (info, " */;\n") && indent_type (info);
 out:
  free (method_type);
  free (context_type);
  return ret;
}

/* Add a static variant to a method.  */

static bool
pr_class_static_method_variant (void *p, const char *physname,
				enum debug_visibility visibility,
				bool constp, bool volatilep)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *method_type;

  assert (info->stack != NULL);
  assert (info->stack->next != NULL);
  assert (info->stack->next->method != NULL);

  /* Put the const and volatile qualifiers on the type.  */
  if (volatilep)
    {
      if (! append_type (info, " volatile"))
	return false;
    }
  if (constp)
    {
      if (! append_type (info, " const"))
	return false;
    }

  /* Mark it as static.  */
  if (! prepend_type (info, "static "))
    return false;

  /* Stick the name of the method into its type.  */
  if (! substitute_type (info, info->stack->next->method))
    return false;

  /* Get the type.  */
  method_type = pop_type (info);
  if (method_type == NULL)
    return false;

  /* Now the top of the stack is the class.  */

  bool ret = (pr_fix_visibility (info, visibility)
	      && append_type (info, method_type)
	      && append_type (info, " /* ")
	      && append_type (info, physname)
	      && append_type (info, " */;\n")
	      && indent_type (info));
  free (method_type);
  return ret;
}

/* Finish up a method.  */

static bool
pr_class_end_method (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  free (info->stack->method);
  info->stack->method = NULL;
  return true;
}

/* Finish up a class.  */

static bool
pr_end_class_type (void *p)
{
  return pr_end_struct_type (p);
}

/* Push a type on the stack using a typedef name.  */

static bool
pr_typedef_type (void *p, const char *name)
{
  struct pr_handle *info = (struct pr_handle *) p;

  return push_type (info, name);
}

/* Push a type on the stack using a tag name.  */

static bool
pr_tag_type (void *p, const char *name, unsigned int id,
	     enum debug_type_kind kind)
{
  struct pr_handle *info = (struct pr_handle *) p;
  const char *t, *tag;
  char idbuf[22];

  switch (kind)
    {
    case DEBUG_KIND_STRUCT:
      t = "struct ";
      break;
    case DEBUG_KIND_UNION:
      t = "union ";
      break;
    case DEBUG_KIND_ENUM:
      t = "enum ";
      break;
    case DEBUG_KIND_CLASS:
      t = "class ";
      break;
    case DEBUG_KIND_UNION_CLASS:
      t = "union class ";
      break;
    default:
      /* PR 25625: Corrupt input can trigger this case.  */
      return false;
    }

  if (! push_type (info, t))
    return false;
  if (name != NULL)
    tag = name;
  else
    {
      sprintf (idbuf, "%%anon%u", id);
      tag = idbuf;
    }

  if (! append_type (info, tag))
    return false;
  if (name != NULL && kind != DEBUG_KIND_ENUM)
    {
      sprintf (idbuf, " /* id %u */", id);
      if (! append_type (info, idbuf))
	return false;
    }

  return true;
}

/* Output a typedef.  */

static bool
pr_typdef (void *p, const char *name)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *s;

  if (! substitute_type (info, name))
    return false;

  s = pop_type (info);
  if (s == NULL)
    return false;

  indent (info);
  fprintf (info->f, "typedef %s;\n", s);

  free (s);

  return true;
}

/* Output a tag.  The tag should already be in the string on the
   stack, so all we have to do here is print it out.  */

static bool
pr_tag (void *p, const char *name ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  t = pop_type (info);
  if (t == NULL)
    return false;

  indent (info);
  fprintf (info->f, "%s;\n", t);

  free (t);

  return true;
}

/* Output an integer constant.  */

static bool
pr_int_constant (void *p, const char *name, bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];

  indent (info);
  print_vma (val, ab, false, false);
  fprintf (info->f, "const int %s = %s;\n", name, ab);
  return true;
}

/* Output a floating point constant.  */

static bool
pr_float_constant (void *p, const char *name, double val)
{
  struct pr_handle *info = (struct pr_handle *) p;

  indent (info);
  fprintf (info->f, "const double %s = %g;\n", name, val);
  return true;
}

/* Output a typed constant.  */

static bool
pr_typed_constant (void *p, const char *name, bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  char ab[22];

  t = pop_type (info);
  if (t == NULL)
    return false;

  indent (info);
  print_vma (val, ab, false, false);
  fprintf (info->f, "const %s %s = %s;\n", t, name, ab);

  free (t);

  return true;
}

/* Output a variable.  */

static bool
pr_variable (void *p, const char *name, enum debug_var_kind kind,
	     bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  char ab[22];

  if (! substitute_type (info, name))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  indent (info);
  switch (kind)
    {
    case DEBUG_STATIC:
    case DEBUG_LOCAL_STATIC:
      fprintf (info->f, "static ");
      break;
    case DEBUG_REGISTER:
      fprintf (info->f, "register ");
      break;
    default:
      break;
    }
  print_vma (val, ab, true, true);
  fprintf (info->f, "%s /* %s */;\n", t, ab);

  free (t);

  return true;
}

/* Start outputting a function.  */

static bool
pr_start_function (void *p, const char *name, bool global)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  if (! substitute_type (info, name))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  indent (info);
  if (! global)
    fprintf (info->f, "static ");
  fprintf (info->f, "%s (", t);

  free (t);

  info->parameter = 1;

  return true;
}

/* Output a function parameter.  */

static bool
pr_function_parameter (void *p, const char *name,
		       enum debug_parm_kind kind, bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  char ab[22];

  if (kind == DEBUG_PARM_REFERENCE
      || kind == DEBUG_PARM_REF_REG)
    {
      if (! pr_reference_type (p))
	return false;
    }

  if (! substitute_type (info, name))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  if (info->parameter != 1)
    fprintf (info->f, ", ");

  if (kind == DEBUG_PARM_REG || kind == DEBUG_PARM_REF_REG)
    fprintf (info->f, "register ");

  print_vma (val, ab, true, true);
  fprintf (info->f, "%s /* %s */", t, ab);

  free (t);

  ++info->parameter;

  return true;
}

/* Start writing out a block.  */

static bool
pr_start_block (void *p, bfd_vma addr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];

  if (info->parameter > 0)
    {
      fprintf (info->f, ")\n");
      info->parameter = 0;
    }

  indent (info);
  print_vma (addr, ab, true, true);
  fprintf (info->f, "{ /* %s */\n", ab);

  info->indent += 2;

  return true;
}

/* Write out line number information.  */

static bool
pr_lineno (void *p, const char *filename, unsigned long lineno, bfd_vma addr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];

  indent (info);
  print_vma (addr, ab, true, true);
  fprintf (info->f, "/* file %s line %lu addr %s */\n", filename, lineno, ab);

  return true;
}

/* Finish writing out a block.  */

static bool
pr_end_block (void *p, bfd_vma addr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];

  info->indent -= 2;

  indent (info);
  print_vma (addr, ab, true, true);
  fprintf (info->f, "} /* %s */\n", ab);

  return true;
}

/* Finish writing out a function.  */

static bool
pr_end_function (void *p ATTRIBUTE_UNUSED)
{
  return true;
}

/* Tags style generation functions start here.  */

/* Variables for address to line translation.  */
static bfd_vma pc;
static const char *filename;
static const char *functionname;
static unsigned int line;
static bool found;

/* Look for an address in a section.  This is called via
   bfd_map_over_sections.  */

static void
find_address_in_section (bfd *abfd, asection *section, void *data)
{
  bfd_vma vma;
  bfd_size_type size;
  asymbol **syms = (asymbol **) data;

  if (found)
    return;

  if ((bfd_section_flags (section) & SEC_ALLOC) == 0)
    return;

  vma = bfd_section_vma (section);
  if (pc < vma)
    return;

  size = bfd_section_size (section);
  if (pc >= vma + size)
    return;

  found = bfd_find_nearest_line (abfd, section, syms, pc - vma,
				 &filename, &functionname, &line);
}

static void
translate_addresses (bfd *abfd, char *addr_hex, FILE *f, asymbol **syms)
{
  pc = bfd_scan_vma (addr_hex, NULL, 16);
  found = false;
  bfd_map_over_sections (abfd, find_address_in_section, syms);

  if (! found)
    fprintf (f, "??");
  else
    fprintf (f, "%u", line);
}

/* Start a new compilation unit.  */

static bool
tg_start_compilation_unit (void * p, const char *fname ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;

  free (info->filename);
  /* Should it be relative? best way to do it here?.  */
  info->filename = xstrdup (fname);

  return true;
}

/* Start a source file within a compilation unit.  */

static bool
tg_start_source (void *p, const char *fname)
{
  struct pr_handle *info = (struct pr_handle *) p;

  free (info->filename);
  /* Should it be relative? best way to do it here?.  */
  info->filename = xstrdup (fname);

  return true;
}

/* Push an enum type onto the type stack.  */

static bool
tg_enum_type (void *p, const char *tag, const char **names,
	      bfd_signed_vma *values)
{
  struct pr_handle *info = (struct pr_handle *) p;
  unsigned int i;
  const char *name;
  char ab[22];

  if (! pr_enum_type (p, tag, names, values))
    return false;

  name = tag ? tag : "unknown";
  /* Generate an entry for the enum.  */
  if (tag)
    fprintf (info->f, "%s\t%s\t0;\"\tkind:e\ttype:%s\n", tag,
	     info->filename, info->stack->type);

  /* Generate entries for the values.  */
  if (names != NULL)
    {
      for (i = 0; names[i] != NULL; i++)
	{
	  print_vma (values[i], ab, false, false);
	  fprintf (info->f, "%s\t%s\t0;\"\tkind:g\tenum:%s\tvalue:%s\n",
		   names[i], info->filename, name, ab);
	}
    }

  return true;
}

/* Start accumulating a struct type.  */

static bool
tg_start_struct_type (void *p, const char *tag, unsigned int id,
		      bool structp,
		      unsigned int size ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;
  const char *name;
  char idbuf[20];

  if (tag != NULL)
    name = tag;
  else
    {
      name = idbuf;
      sprintf (idbuf, "%%anon%u", id);
    }

  if (! push_type (info, name))
    return false;

  info->stack->flavor = structp ? "struct" : "union";

  fprintf (info->f, "%s\t%s\t0;\"\tkind:%c\n", name, info->filename,
	   info->stack->flavor[0]);

  info->stack->visibility = DEBUG_VISIBILITY_PUBLIC;

  return indent_type (info);
}

/* Output the visibility of a field in a struct.  */

static bool
tg_fix_visibility (struct pr_handle *info, enum debug_visibility visibility)
{
  assert (info->stack != NULL);

  if (info->stack->visibility == visibility)
    return true;

  assert (info->stack->visibility != DEBUG_VISIBILITY_IGNORE);

  info->stack->visibility = visibility;

  return true;
}

/* Add a field to a struct type.  */

static bool
tg_struct_field (void *p, const char *name, bfd_vma bitpos ATTRIBUTE_UNUSED,
		 bfd_vma bitsize ATTRIBUTE_UNUSED,
		 enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  t = pop_type (info);
  if (t == NULL)
    return false;

  if (! tg_fix_visibility (info, visibility))
    {
      free (t);
      return false;
    }

  /* It happens, a bug? */
  if (! name[0])
    {
      free (t);
      return true;
    }

  fprintf (info->f, "%s\t%s\t0;\"\tkind:m\ttype:%s\t%s:%s\taccess:%s\n",
	   name, info->filename, t, info->stack->flavor, info->stack->type,
	   visibility_name (visibility));

  free (t);

  return true;
}

/* Finish a struct type.  */

static bool
tg_end_struct_type (void *p ATTRIBUTE_UNUSED)
{
  assert (((struct pr_handle *) p)->stack != NULL);

  return true;
}

/* Start a class type.  */

static bool
tg_start_class_type (void *p, const char *tag, unsigned int id,
		     bool structp, unsigned int size,
		     bool vptr, bool ownvptr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *tv = NULL;
  const char *name;
  char idbuf[20];
  bool ret = false;

  info->indent += 2;

  if (vptr && ! ownvptr)
    {
      tv = pop_type (info);
      if (tv == NULL)
	return false;
    }

  if (tag != NULL)
    name = tag;
  else
    {
      sprintf (idbuf, "%%anon%u", id);
      name = idbuf;
    }

  if (! push_type (info, name))
    goto out;

  info->stack->flavor = structp ? "class" : "union class";
  free (info->stack->parents);
  info->stack->parents = NULL;

  if (size != 0 || vptr || ownvptr || tag != NULL)
    {
      if (vptr)
	{
	  if (! append_type (info, " vtable "))
	    goto out;
	  if (ownvptr)
	    {
	      if (! append_type (info, "self "))
		goto out;
	    }
	  else
	    {
	      if (! append_type (info, tv)
		  || ! append_type (info, " "))
		goto out;
	    }
	}
    }

  info->stack->visibility = DEBUG_VISIBILITY_PRIVATE;

  ret = true;
 out:
  free (tv);
  return ret;
}

/* Add a static member to a class.  */

static bool
tg_class_static_member (void *p, const char *name,
			const char *physname ATTRIBUTE_UNUSED,
			enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  int len_var, len_class;
  char *full_name;

  len_var = strlen (name);
  len_class = strlen (info->stack->next->type);
  full_name = xmalloc (len_var + len_class + 3);
  sprintf (full_name, "%s::%s", info->stack->next->type, name);

  if (! substitute_type (info, full_name))
    {
      free (full_name);
      return false;
    }

  if (! prepend_type (info, "static "))
    {
      free (full_name);
      return false;
    }

  t = pop_type (info);
  if (t == NULL)
    {
      free (full_name);
      return false;
    }

  if (! tg_fix_visibility (info, visibility))
    {
      free (t);
      free (full_name);
      return false;
    }

  fprintf (info->f, "%s\t%s\t0;\"\tkind:x\ttype:%s\tclass:%s\taccess:%s\n",
	   name, info->filename, t, info->stack->type,
	   visibility_name (visibility));
  free (t);
  free (full_name);

  return true;
}

/* Add a base class to a class.  */

static bool
tg_class_baseclass (void *p, bfd_vma bitpos ATTRIBUTE_UNUSED,
		    bool is_virtual, enum debug_visibility visibility)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  const char *prefix;

  assert (info->stack != NULL && info->stack->next != NULL);

  t = pop_type (info);
  if (t == NULL)
    return false;

  /* Push it back on to take advantage of the prepend_type and
     append_type routines.  */
  if (! push_type (info, t + (startswith (t, "class ")
			      ? sizeof "class " - 1 : 0)))
    {
      free (t);
      return false;
    }
  free (t);

  if (is_virtual)
    {
      if (! prepend_type (info, "virtual "))
	return false;
    }

  switch (visibility)
    {
    case DEBUG_VISIBILITY_PUBLIC:
      prefix = "public ";
      break;
    case DEBUG_VISIBILITY_PROTECTED:
      prefix = "protected ";
      break;
    case DEBUG_VISIBILITY_PRIVATE:
      prefix = "private ";
      break;
    default:
      prefix = "/* unknown visibility */ ";
      break;
    }

  if (! prepend_type (info, prefix))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  bool ret = ((!info->stack->parents || append_parent (info, ", "))
	      && append_parent (info, t));
  free (t);
  return ret;
}

/* Add a variant to a method.  */

static bool
tg_class_method_variant (void *p, const char *physname ATTRIBUTE_UNUSED,
			 enum debug_visibility visibility,
			 bool constp, bool volatilep,
			 bfd_vma voffset ATTRIBUTE_UNUSED,
			 bool context)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *method_type;
  char *context_type;
  char *method_name;

  assert (info->stack != NULL);
  assert (info->stack->next != NULL);

  /* Put the const and volatile qualifiers on the type.  */
  if (volatilep)
    {
      if (! append_type (info, " volatile"))
	return false;
    }
  if (constp)
    {
      if (! append_type (info, " const"))
	return false;
    }

  method_name = strdup (context ? info->stack->next->next->method
			: info->stack->next->method);

  /* Stick the name of the method into its type.  */
  if (! substitute_type (info, method_name))
    {
      free (method_name);
      return false;
    }

  /* Get the type.  */
  method_type = pop_type (info);
  if (method_type == NULL)
    {
      free (method_name);
      return false;
    }

  /* Pull off the context type if there is one.  */
  if (! context)
    context_type = NULL;
  else
    {
      context_type = pop_type (info);
      if (context_type == NULL)
	{
	  free (method_type);
	  free (method_name);
	  return false;
	}
    }

  /* Now the top of the stack is the class.  */
  if (! tg_fix_visibility (info, visibility))
    {
      free (method_type);
      free (method_name);
      free (context_type);
      return false;
    }

  fprintf (info->f, "%s\t%s\t0;\"\tkind:p\ttype:%s\tclass:%s\n",
	   method_name, info->filename, method_type, info->stack->type);
  free (method_type);
  free (method_name);
  free (context_type);

  return true;
}

/* Add a static variant to a method.  */

static bool
tg_class_static_method_variant (void *p,
				const char *physname ATTRIBUTE_UNUSED,
				enum debug_visibility visibility,
				bool constp, bool volatilep)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *method_type;
  char *method_name;

  assert (info->stack != NULL);
  assert (info->stack->next != NULL);
  assert (info->stack->next->method != NULL);

  /* Put the const and volatile qualifiers on the type.  */
  if (volatilep)
    {
      if (! append_type (info, " volatile"))
	return false;
    }
  if (constp)
    {
      if (! append_type (info, " const"))
	return false;
    }

  /* Mark it as static.  */
  if (! prepend_type (info, "static "))
    return false;

  method_name = strdup (info->stack->next->method);
  /* Stick the name of the method into its type.  */
  if (! substitute_type (info, info->stack->next->method))
    {
      free (method_name);
      return false;
    }

  /* Get the type.  */
  method_type = pop_type (info);
  if (method_type == NULL)
    {
      free (method_name);
      return false;
    }

  /* Now the top of the stack is the class.  */
  if (! tg_fix_visibility (info, visibility))
    {
      free (method_type);
      free (method_name);
      return false;
    }

  fprintf (info->f, "%s\t%s\t0;\"\tkind:p\ttype:%s\tclass:%s\taccess:%s\n",
	   method_name, info->filename, method_type, info->stack->type,
	   visibility_name (visibility));
  free (method_type);
  free (method_name);

  return true;
}

/* Finish up a class.  */

static bool
tg_end_class_type (void *p)
{
  struct pr_handle *info = (struct pr_handle *) p;

  fprintf (info->f, "%s\t%s\t0;\"\tkind:c\ttype:%s", info->stack->type,
	   info->filename, info->stack->flavor);
  if (info->stack->parents)
    {
      fprintf  (info->f, "\tinherits:%s", info->stack->parents);
      free (info->stack->parents);
      info->stack->parents = NULL;
    }
  fputc ('\n', info->f);

  return tg_end_struct_type (p);
}

/* Push a type on the stack using a tag name.  */

static bool
tg_tag_type (void *p, const char *name, unsigned int id,
	     enum debug_type_kind kind)
{
  struct pr_handle *info = (struct pr_handle *) p;
  const char *t, *tag;
  char idbuf[20];

  switch (kind)
    {
    case DEBUG_KIND_STRUCT:
      t = "struct ";
      break;
    case DEBUG_KIND_UNION:
      t = "union ";
      break;
    case DEBUG_KIND_ENUM:
      t = "enum ";
      break;
    case DEBUG_KIND_CLASS:
      t = "class ";
      break;
    case DEBUG_KIND_UNION_CLASS:
      t = "union class ";
      break;
    default:
      abort ();
      return false;
    }

  if (! push_type (info, t))
    return false;
  if (name != NULL)
    tag = name;
  else
    {
      sprintf (idbuf, "%%anon%u", id);
      tag = idbuf;
    }

  if (! append_type (info, tag))
    return false;

  return true;
}

/* Output a typedef.  */

static bool
tg_typdef (void *p, const char *name)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *s;

  s = pop_type (info);
  if (s == NULL)
    return false;

  fprintf (info->f, "%s\t%s\t0;\"\tkind:t\ttype:%s\n", name,
	   info->filename, s);

  free (s);

  return true;
}

/* Output a tag.  The tag should already be in the string on the
   stack, so all we have to do here is print it out.  */

static bool
tg_tag (void *p ATTRIBUTE_UNUSED, const char *name ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  t = pop_type (info);
  if (t == NULL)
    return false;
  free (t);

  return true;
}

/* Output an integer constant.  */

static bool
tg_int_constant (void *p, const char *name, bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22];

  indent (info);
  print_vma (val, ab, false, false);
  fprintf (info->f, "%s\t%s\t0;\"\tkind:v\ttype:const int\tvalue:%s\n",
	   name, info->filename, ab);
  return true;
}

/* Output a floating point constant.  */

static bool
tg_float_constant (void *p, const char *name, double val)
{
  struct pr_handle *info = (struct pr_handle *) p;

  indent (info);
  fprintf (info->f, "%s\t%s\t0;\"\tkind:v\ttype:const double\tvalue:%g\n",
	   name, info->filename, val);
  return true;
}

/* Output a typed constant.  */

static bool
tg_typed_constant (void *p, const char *name, bfd_vma val)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;
  char ab[22];

  t = pop_type (info);
  if (t == NULL)
    return false;

  indent (info);
  print_vma (val, ab, false, false);
  fprintf (info->f, "%s\t%s\t0;\"\tkind:v\ttype:const %s\tvalue:%s\n",
	   name, info->filename, t, ab);

  free (t);

  return true;
}

/* Output a variable.  */

static bool
tg_variable (void *p, const char *name, enum debug_var_kind kind,
	     bfd_vma val ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t, *dname, *from_class;

  t = pop_type (info);
  if (t == NULL)
    return false;

  dname = NULL;
  if (info->demangler)
    dname = info->demangler (info->abfd, name, demangle_flags);

  from_class = NULL;
  if (dname != NULL)
    {
      char *sep;
      sep = strstr (dname, "::");
      if (sep)
	{
	  *sep = 0;
	  name = sep + 2;
	  from_class = dname;
	}
      else
	/* Obscure types as vts and type_info nodes.  */
	name = dname;
    }

  fprintf (info->f, "%s\t%s\t0;\"\tkind:v\ttype:%s", name, info->filename, t);

  switch (kind)
    {
    case DEBUG_STATIC:
    case DEBUG_LOCAL_STATIC:
      fprintf (info->f, "\tfile:");
      break;
    case DEBUG_REGISTER:
      fprintf (info->f, "\tregister:");
      break;
    default:
      break;
    }

  if (from_class)
    fprintf (info->f, "\tclass:%s", from_class);

  if (dname)
    free (dname);

  fprintf (info->f, "\n");

  free (t);

  return true;
}

/* Start outputting a function.  */

static bool
tg_start_function (void *p, const char *name, bool global)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *dname;

  if (! global)
    info->stack->flavor = "static";
  else
    info->stack->flavor = NULL;

  dname = NULL;
  if (info->demangler)
    dname = info->demangler (info->abfd, name, demangle_flags);

  if (! substitute_type (info, dname ? dname : name))
    return false;

  free (info->stack->method);
  info->stack->method = NULL;
  if (dname != NULL)
    {
      char *sep;
      sep = strstr (dname, "::");
      if (sep)
	{
	  info->stack->method = dname;
	  dname = NULL;
	  *sep = 0;
	  name = sep + 2;
	}
      else
	{
	  info->stack->method = xstrdup ("");
	  name = dname;
	}
      sep = strchr (name, '(');
      if (sep)
	*sep = 0;
      /* Obscure functions as type_info function.  */
    }

  free (info->stack->parents);
  info->stack->parents = strdup (name);
  free (dname);

  if (! info->stack->method && ! append_type (info, "("))
    return false;

  info->parameter = 1;

  return true;
}

/* Output a function parameter.  */

static bool
tg_function_parameter (void *p, const char *name, enum debug_parm_kind kind,
		       bfd_vma val ATTRIBUTE_UNUSED)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char *t;

  if (kind == DEBUG_PARM_REFERENCE
      || kind == DEBUG_PARM_REF_REG)
    {
      if (! pr_reference_type (p))
	return false;
    }

  if (! substitute_type (info, name))
    return false;

  t = pop_type (info);
  if (t == NULL)
    return false;

  if (! info->stack->method)
    {
      if (info->parameter != 1 && ! append_type (info, ", "))
	{
	  free (t);
	  return false;
	}

      if (kind == DEBUG_PARM_REG || kind == DEBUG_PARM_REF_REG)
	if (! append_type (info, "register "))
	  {
	    free (t);
	    return false;
	  }

      if (! append_type (info, t))
	{
	  free (t);
	  return false;
	}
    }

  free (t);

  ++info->parameter;

  return true;
}

/* Start writing out a block.  */

static bool
tg_start_block (void *p, bfd_vma addr)
{
  struct pr_handle *info = (struct pr_handle *) p;
  char ab[22], kind, *partof;
  char *t;
  bool local;

  if (info->parameter > 0)
    {
      info->parameter = 0;

      /* Delayed name.  */
      fprintf (info->f, "%s\t%s\t", info->stack->parents, info->filename);
      free (info->stack->parents);
      info->stack->parents = NULL;

      print_vma (addr, ab, true, true);
      translate_addresses (info->abfd, ab, info->f, info->syms);
      local = info->stack->flavor != NULL;
      if (info->stack->method && *info->stack->method)
	{
	  kind = 'm';
	  partof = (char *) info->stack->method;
	}
      else
	{
	  kind = 'f';
	  partof = NULL;
	  if (! info->stack->method && ! append_type (info, ")"))
	    return false;
	}
      t = pop_type (info);
      if (t == NULL)
	return false;
      fprintf (info->f, ";\"\tkind:%c\ttype:%s", kind, t);
      free (t);
      if (local)
	fputs ("\tfile:", info->f);
      if (partof)
	fprintf (info->f, "\tclass:%s", partof);
      fputc ('\n', info->f);
      free (info->stack->method);
      info->stack->method = NULL;
    }

  return true;
}

/* Write out line number information.  */

static bool
tg_lineno (void *p ATTRIBUTE_UNUSED, const char *fname ATTRIBUTE_UNUSED,
	   unsigned long lineno ATTRIBUTE_UNUSED,
	   bfd_vma addr ATTRIBUTE_UNUSED)
{
  return true;
}

/* Finish writing out a block.  */

static bool
tg_end_block (void *p ATTRIBUTE_UNUSED, bfd_vma addr ATTRIBUTE_UNUSED)
{
  return true;
}

/* Convert the visibility value into a human readable name.  */

static const char *
visibility_name (enum debug_visibility visibility)
{
  const char *s;

  switch (visibility)
    {
    case DEBUG_VISIBILITY_PUBLIC:
      s = "public";
      break;
    case DEBUG_VISIBILITY_PRIVATE:
      s = "private";
      break;
    case DEBUG_VISIBILITY_PROTECTED:
      s = "protected";
      break;
    case DEBUG_VISIBILITY_IGNORE:
      s = "/* ignore */";
      break;
    default:
      abort ();
      return false;
    }
  return s;
}
