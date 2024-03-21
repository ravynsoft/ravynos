/* ELF attributes support (based on ARM EABI attributes).
   Copyright (C) 2005-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "elf-bfd.h"

/* Return the number of bytes needed by I in uleb128 format.  */
static int
uleb128_size (unsigned int i)
{
  int size;
  size = 1;
  while (i >= 0x80)
    {
      i >>= 7;
      size++;
    }
  return size;
}

/* Return TRUE if the attribute has the default value (0/"").  */
static bool
is_default_attr (obj_attribute *attr)
{
  if (ATTR_TYPE_HAS_ERROR (attr->type))
    return true;
  if (ATTR_TYPE_HAS_INT_VAL (attr->type) && attr->i != 0)
    return false;
  if (ATTR_TYPE_HAS_STR_VAL (attr->type) && attr->s && *attr->s)
    return false;
  if (ATTR_TYPE_HAS_NO_DEFAULT (attr->type))
    return false;

  return true;
}

/* Return the size of a single attribute.  */
static bfd_vma
obj_attr_size (unsigned int tag, obj_attribute *attr)
{
  bfd_vma size;

  if (is_default_attr (attr))
    return 0;

  size = uleb128_size (tag);
  if (ATTR_TYPE_HAS_INT_VAL (attr->type))
    size += uleb128_size (attr->i);
  if (ATTR_TYPE_HAS_STR_VAL (attr->type))
    size += strlen ((char *)attr->s) + 1;
  return size;
}

/* Return the vendor name for a given object attributes section.  */
static const char *
vendor_obj_attr_name (bfd *abfd, int vendor)
{
  return (vendor == OBJ_ATTR_PROC
	  ? get_elf_backend_data (abfd)->obj_attrs_vendor
	  : "gnu");
}

/* Return the size of the object attributes section for VENDOR
   (OBJ_ATTR_PROC or OBJ_ATTR_GNU), or 0 if there are no attributes
   for that vendor to record and the vendor is OBJ_ATTR_GNU.  */
static bfd_vma
vendor_obj_attr_size (bfd *abfd, int vendor)
{
  bfd_vma size;
  obj_attribute *attr;
  obj_attribute_list *list;
  int i;
  const char *vendor_name = vendor_obj_attr_name (abfd, vendor);

  if (!vendor_name)
    return 0;

  attr = elf_known_obj_attributes (abfd)[vendor];
  size = 0;
  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    size += obj_attr_size (i, &attr[i]);

  for (list = elf_other_obj_attributes (abfd)[vendor];
       list;
       list = list->next)
    size += obj_attr_size (list->tag, &list->attr);

  /* <size> <vendor_name> NUL 0x1 <size> */
  return (size
	  ? size + 10 + strlen (vendor_name)
	  : 0);
}

/* Return the size of the object attributes section.  */
bfd_vma
bfd_elf_obj_attr_size (bfd *abfd)
{
  bfd_vma size;

  size = vendor_obj_attr_size (abfd, OBJ_ATTR_PROC);
  size += vendor_obj_attr_size (abfd, OBJ_ATTR_GNU);

  /* 'A' <sections for each vendor> */
  return (size ? size + 1 : 0);
}

/* Write VAL in uleb128 format to P, returning a pointer to the
   following byte.  */
static bfd_byte *
write_uleb128 (bfd_byte *p, unsigned int val)
{
  bfd_byte c;
  do
    {
      c = val & 0x7f;
      val >>= 7;
      if (val)
	c |= 0x80;
      *(p++) = c;
    }
  while (val);
  return p;
}

/* Write attribute ATTR to butter P, and return a pointer to the following
   byte.  */
static bfd_byte *
write_obj_attribute (bfd_byte *p, unsigned int tag, obj_attribute *attr)
{
  /* Suppress default entries.  */
  if (is_default_attr (attr))
    return p;

  p = write_uleb128 (p, tag);
  if (ATTR_TYPE_HAS_INT_VAL (attr->type))
    p = write_uleb128 (p, attr->i);
  if (ATTR_TYPE_HAS_STR_VAL (attr->type))
    {
      int len;

      len = strlen (attr->s) + 1;
      memcpy (p, attr->s, len);
      p += len;
    }

  return p;
}

/* Write the contents of the object attributes section (length SIZE)
   for VENDOR to CONTENTS.  */
static void
vendor_set_obj_attr_contents (bfd *abfd, bfd_byte *contents, bfd_vma size,
			      int vendor)
{
  bfd_byte *p;
  obj_attribute *attr;
  obj_attribute_list *list;
  int i;
  const char *vendor_name = vendor_obj_attr_name (abfd, vendor);
  size_t vendor_length = strlen (vendor_name) + 1;

  p = contents;
  bfd_put_32 (abfd, size, p);
  p += 4;
  memcpy (p, vendor_name, vendor_length);
  p += vendor_length;
  *(p++) = Tag_File;
  bfd_put_32 (abfd, size - 4 - vendor_length, p);
  p += 4;

  attr = elf_known_obj_attributes (abfd)[vendor];
  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    {
      unsigned int tag = i;
      if (get_elf_backend_data (abfd)->obj_attrs_order)
	tag = get_elf_backend_data (abfd)->obj_attrs_order (i);
      p = write_obj_attribute (p, tag, &attr[tag]);
    }

  for (list = elf_other_obj_attributes (abfd)[vendor];
       list;
       list = list->next)
    p = write_obj_attribute (p, list->tag, &list->attr);
}

/* Write the contents of the object attributes section to CONTENTS.  */
void
bfd_elf_set_obj_attr_contents (bfd *abfd, bfd_byte *contents, bfd_vma size)
{
  bfd_byte *p;
  int vendor;
  bfd_vma my_size;

  p = contents;
  *(p++) = 'A';
  my_size = 1;
  for (vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; vendor++)
    {
      bfd_vma vendor_size = vendor_obj_attr_size (abfd, vendor);
      if (vendor_size)
	vendor_set_obj_attr_contents (abfd, p, vendor_size, vendor);
      p += vendor_size;
      my_size += vendor_size;
    }

  if (size != my_size)
    abort ();
}

/* Allocate/find an object attribute.  */
static obj_attribute *
elf_new_obj_attr (bfd *abfd, int vendor, unsigned int tag)
{
  obj_attribute *attr;
  obj_attribute_list *list;
  obj_attribute_list *p;
  obj_attribute_list **lastp;


  if (tag < NUM_KNOWN_OBJ_ATTRIBUTES)
    {
      /* Known tags are preallocated.  */
      attr = &elf_known_obj_attributes (abfd)[vendor][tag];
    }
  else
    {
      /* Create a new tag.  */
      list = (obj_attribute_list *)
	bfd_alloc (abfd, sizeof (obj_attribute_list));
      memset (list, 0, sizeof (obj_attribute_list));
      list->tag = tag;
      /* Keep the tag list in order.  */
      lastp = &elf_other_obj_attributes (abfd)[vendor];
      for (p = *lastp; p; p = p->next)
	{
	  if (tag < p->tag)
	    break;
	  lastp = &p->next;
	}
      list->next = *lastp;
      *lastp = list;
      attr = &list->attr;
    }

  return attr;
}

/* Return the value of an integer object attribute.  */
int
bfd_elf_get_obj_attr_int (bfd *abfd, int vendor, unsigned int tag)
{
  obj_attribute_list *p;

  if (tag < NUM_KNOWN_OBJ_ATTRIBUTES)
    {
      /* Known tags are preallocated.  */
      return elf_known_obj_attributes (abfd)[vendor][tag].i;
    }
  else
    {
      for (p = elf_other_obj_attributes (abfd)[vendor];
	   p;
	   p = p->next)
	{
	  if (tag == p->tag)
	    return p->attr.i;
	  if (tag < p->tag)
	    break;
	}
      return 0;
    }
}

/* Add an integer object attribute.  */
void
bfd_elf_add_obj_attr_int (bfd *abfd, int vendor, unsigned int tag, unsigned int i)
{
  obj_attribute *attr;

  attr = elf_new_obj_attr (abfd, vendor, tag);
  attr->type = _bfd_elf_obj_attrs_arg_type (abfd, vendor, tag);
  attr->i = i;
}

/* Duplicate an object attribute string value.  */
static char *
elf_attr_strdup (bfd *abfd, const char *s, const char *end)
{
  char *p;
  size_t len;

  if (end)
    len = strnlen (s, end - s);
  else
    len = strlen (s);

  p = (char *) bfd_alloc (abfd, len + 1);
  if (p != NULL)
    {
      memcpy (p, s, len);
      p[len] = 0;
    }
  return p;
}

char *
_bfd_elf_attr_strdup (bfd *abfd, const char *s)
{
  return elf_attr_strdup (abfd, s, NULL);
}

/* Add a string object attribute.  */
static void
elf_add_obj_attr_string (bfd *abfd, int vendor, unsigned int tag,
			 const char *s, const char *end)
{
  obj_attribute *attr;

  attr = elf_new_obj_attr (abfd, vendor, tag);
  attr->type = _bfd_elf_obj_attrs_arg_type (abfd, vendor, tag);
  attr->s = elf_attr_strdup (abfd, s, end);
}

void
bfd_elf_add_obj_attr_string (bfd *abfd, int vendor, unsigned int tag,
			     const char *s)
{
  elf_add_obj_attr_string (abfd, vendor, tag, s, NULL);
}

/* Add a int+string object attribute.  */
static void
elf_add_obj_attr_int_string (bfd *abfd, int vendor, unsigned int tag,
			     unsigned int i, const char *s, const char *end)
{
  obj_attribute *attr;

  attr = elf_new_obj_attr (abfd, vendor, tag);
  attr->type = _bfd_elf_obj_attrs_arg_type (abfd, vendor, tag);
  attr->i = i;
  attr->s = elf_attr_strdup (abfd, s, end);
}

void
bfd_elf_add_obj_attr_int_string (bfd *abfd, int vendor, unsigned int tag,
				 unsigned int i, const char *s)
{
  elf_add_obj_attr_int_string (abfd, vendor, tag, i, s, NULL);
}

/* Copy the object attributes from IBFD to OBFD.  */
void
_bfd_elf_copy_obj_attributes (bfd *ibfd, bfd *obfd)
{
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  obj_attribute_list *list;
  int i;
  int vendor;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return;

  for (vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; vendor++)
    {
      in_attr
	= &elf_known_obj_attributes (ibfd)[vendor][LEAST_KNOWN_OBJ_ATTRIBUTE];
      out_attr
	= &elf_known_obj_attributes (obfd)[vendor][LEAST_KNOWN_OBJ_ATTRIBUTE];
      for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
	{
	  out_attr->type = in_attr->type;
	  out_attr->i = in_attr->i;
	  if (in_attr->s && *in_attr->s)
	    out_attr->s = _bfd_elf_attr_strdup (obfd, in_attr->s);
	  in_attr++;
	  out_attr++;
	}

      for (list = elf_other_obj_attributes (ibfd)[vendor];
	   list;
	   list = list->next)
	{
	  in_attr = &list->attr;
	  switch (in_attr->type & (ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL))
	    {
	    case ATTR_TYPE_FLAG_INT_VAL:
	      bfd_elf_add_obj_attr_int (obfd, vendor, list->tag, in_attr->i);
	      break;
	    case ATTR_TYPE_FLAG_STR_VAL:
	      bfd_elf_add_obj_attr_string (obfd, vendor, list->tag,
					   in_attr->s);
	      break;
	    case ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL:
	      bfd_elf_add_obj_attr_int_string (obfd, vendor, list->tag,
					       in_attr->i, in_attr->s);
	      break;
	    default:
	      abort ();
	    }
	}
    }
}

/* Determine whether a GNU object attribute tag takes an integer, a
   string or both.  */
static int
gnu_obj_attrs_arg_type (unsigned int tag)
{
  /* Except for Tag_compatibility, for GNU attributes we follow the
     same rule ARM ones > 32 follow: odd-numbered tags take strings
     and even-numbered tags take integers.  In addition, tag & 2 is
     nonzero for architecture-independent tags and zero for
     architecture-dependent ones.  */
  if (tag == Tag_compatibility)
    return 3;
  else
    return (tag & 1) != 0 ? 2 : 1;
}

/* Determine what arguments an attribute tag takes.  */
int
_bfd_elf_obj_attrs_arg_type (bfd *abfd, int vendor, unsigned int tag)
{
  switch (vendor)
    {
    case OBJ_ATTR_PROC:
      return get_elf_backend_data (abfd)->obj_attrs_arg_type (tag);
      break;
    case OBJ_ATTR_GNU:
      return gnu_obj_attrs_arg_type (tag);
      break;
    default:
      abort ();
    }
}

/* Parse an object attributes section.  */
void
_bfd_elf_parse_attributes (bfd *abfd, Elf_Internal_Shdr * hdr)
{
  bfd_byte *contents;
  bfd_byte *p;
  bfd_byte *p_end;
  const char *std_sec;
  ufile_ptr filesize;

  /* PR 17512: file: 2844a11d.  */
  if (hdr->sh_size == 0)
    return;

  filesize = bfd_get_file_size (abfd);
  if (filesize != 0 && hdr->sh_size > filesize)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: error: attribute section '%pA' too big: %#llx"),
			  abfd, hdr->bfd_section, (long long) hdr->sh_size);
      bfd_set_error (bfd_error_invalid_operation);
      return;
    }

  contents = (bfd_byte *) bfd_malloc (hdr->sh_size);
  if (!contents)
    return;
  if (!bfd_get_section_contents (abfd, hdr->bfd_section, contents, 0,
				 hdr->sh_size))
    {
      free (contents);
      return;
    }
  p = contents;
  p_end = p + hdr->sh_size;
  std_sec = get_elf_backend_data (abfd)->obj_attrs_vendor;

  if (*p++ == 'A')
    {
      while (p_end - p >= 4)
	{
	  size_t len = p_end - p;
	  size_t namelen;
	  size_t section_len;
	  int vendor;

	  section_len = bfd_get_32 (abfd, p);
	  p += 4;
	  if (section_len == 0)
	    break;
	  if (section_len > len)
	    section_len = len;
	  if (section_len <= 4)
	    {
	      _bfd_error_handler
		(_("%pB: error: attribute section length too small: %ld"),
		 abfd, (long) section_len);
	      break;
	    }
	  section_len -= 4;
	  namelen = strnlen ((char *) p, section_len) + 1;
	  if (namelen >= section_len)
	    break;
	  if (std_sec && strcmp ((char *) p, std_sec) == 0)
	    vendor = OBJ_ATTR_PROC;
	  else if (strcmp ((char *) p, "gnu") == 0)
	    vendor = OBJ_ATTR_GNU;
	  else
	    {
	      /* Other vendor section.  Ignore it.  */
	      p += section_len;
	      continue;
	    }

	  p += namelen;
	  section_len -= namelen;
	  while (section_len > 0)
	    {
	      unsigned int tag;
	      unsigned int val;
	      size_t subsection_len;
	      bfd_byte *end, *orig_p;

	      orig_p = p;
	      tag = _bfd_safe_read_leb128 (abfd, &p, false, p_end);
	      if (p_end - p >= 4)
		{
		  subsection_len = bfd_get_32 (abfd, p);
		  p += 4;
		}
	      else
		{
		  p = p_end;
		  break;
		}
	      if (subsection_len > section_len)
		subsection_len = section_len;
	      section_len -= subsection_len;
	      end = orig_p + subsection_len;
	      if (end < p)
		break;
	      switch (tag)
		{
		case Tag_File:
		  while (p < end)
		    {
		      int type;

		      tag = _bfd_safe_read_leb128 (abfd, &p, false, end);
		      type = _bfd_elf_obj_attrs_arg_type (abfd, vendor, tag);
		      switch (type & (ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL))
			{
			case ATTR_TYPE_FLAG_INT_VAL | ATTR_TYPE_FLAG_STR_VAL:
			  val = _bfd_safe_read_leb128 (abfd, &p, false, end);
			  elf_add_obj_attr_int_string (abfd, vendor, tag, val,
						       (char *) p,
						       (char *) end);
			  p += strnlen ((char *) p, end - p);
			  if (p < end)
			    p++;
			  break;
			case ATTR_TYPE_FLAG_STR_VAL:
			  elf_add_obj_attr_string (abfd, vendor, tag,
						   (char *) p,
						   (char *) end);
			  p += strnlen ((char *) p, end - p);
			  if (p < end)
			    p++;
			  break;
			case ATTR_TYPE_FLAG_INT_VAL:
			  val = _bfd_safe_read_leb128 (abfd, &p, false, end);
			  bfd_elf_add_obj_attr_int (abfd, vendor, tag, val);
			  break;
			default:
			  abort ();
			}
		    }
		  break;
		case Tag_Section:
		case Tag_Symbol:
		  /* Don't have anywhere convenient to attach these.
		     Fall through for now.  */
		default:
		  /* Ignore things we don't know about.  */
		  p = end;
		  break;
		}
	    }
	}
    }
  free (contents);
}

/* Merge common object attributes from IBFD into OBFD.  Raise an error
   if there are conflicting attributes.  Any processor-specific
   attributes have already been merged.  This must be called from the
   bfd_elfNN_bfd_merge_private_bfd_data hook for each individual
   target, along with any target-specific merging.  Because there are
   no common attributes other than Tag_compatibility at present, and
   non-"gnu" Tag_compatibility is not expected in "gnu" sections, this
   is not presently called for targets without their own
   attributes.  */

bool
_bfd_elf_merge_object_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  int vendor;

  /* The only common attribute is currently Tag_compatibility,
     accepted in both processor and "gnu" sections.  */
  for (vendor = OBJ_ATTR_FIRST; vendor <= OBJ_ATTR_LAST; vendor++)
    {
      /* Handle Tag_compatibility.  The tags are only compatible if the flags
	 are identical and, if the flags are '1', the strings are identical.
	 If the flags are non-zero, then we can only use the string "gnu".  */
      in_attr = &elf_known_obj_attributes (ibfd)[vendor][Tag_compatibility];
      out_attr = &elf_known_obj_attributes (obfd)[vendor][Tag_compatibility];

      if (in_attr->i > 0 && strcmp (in_attr->s, "gnu") != 0)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
		(_("error: %pB: object has vendor-specific contents that "
		   "must be processed by the '%s' toolchain"),
		 ibfd, in_attr->s);
	  return false;
	}

      if (in_attr->i != out_attr->i
	  || (in_attr->i != 0 && strcmp (in_attr->s, out_attr->s) != 0))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("error: %pB: object tag '%d, %s' is "
				"incompatible with tag '%d, %s'"),
			      ibfd,
			      in_attr->i, in_attr->s ? in_attr->s : "",
			      out_attr->i, out_attr->s ? out_attr->s : "");
	  return false;
	}
    }

  return true;
}

/* Merge an unknown processor-specific attribute TAG, within the range
   of known attributes, from IBFD into OBFD; return TRUE if the link
   is OK, FALSE if it must fail.  */

bool
_bfd_elf_merge_unknown_attribute_low (bfd *ibfd, bfd *obfd, int tag)
{
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  bfd *err_bfd = NULL;
  bool result = true;

  in_attr = elf_known_obj_attributes_proc (ibfd);
  out_attr = elf_known_obj_attributes_proc (obfd);

  if (out_attr[tag].i != 0 || out_attr[tag].s != NULL)
    err_bfd = obfd;
  else if (in_attr[tag].i != 0 || in_attr[tag].s != NULL)
    err_bfd = ibfd;

  if (err_bfd != NULL)
    result
      = get_elf_backend_data (err_bfd)->obj_attrs_handle_unknown (err_bfd, tag);

  /* Only pass on attributes that match in both inputs.  */
  if (in_attr[tag].i != out_attr[tag].i
      || (in_attr[tag].s == NULL) != (out_attr[tag].s == NULL)
      || (in_attr[tag].s != NULL && out_attr[tag].s != NULL
	  && strcmp (in_attr[tag].s, out_attr[tag].s) != 0))
    {
      out_attr[tag].i = 0;
      out_attr[tag].s = NULL;
    }

  return result;
}

/* Merge the lists of unknown processor-specific attributes, outside
   the known range, from IBFD into OBFD; return TRUE if the link is
   OK, FALSE if it must fail.  */

bool
_bfd_elf_merge_unknown_attribute_list (bfd *ibfd, bfd *obfd)
{
  obj_attribute_list *in_list;
  obj_attribute_list *out_list;
  obj_attribute_list **out_listp;
  bool result = true;

  in_list = elf_other_obj_attributes_proc (ibfd);
  out_listp = &elf_other_obj_attributes_proc (obfd);
  out_list = *out_listp;

  for (; in_list || out_list; )
    {
      bfd *err_bfd = NULL;
      unsigned int err_tag = 0;

      /* The tags for each list are in numerical order.  */
      /* If the tags are equal, then merge.  */
      if (out_list && (!in_list || in_list->tag > out_list->tag))
	{
	  /* This attribute only exists in obfd.  We can't merge, and we don't
	     know what the tag means, so delete it.  */
	  err_bfd = obfd;
	  err_tag = out_list->tag;
	  *out_listp = out_list->next;
	  out_list = *out_listp;
	}
      else if (in_list && (!out_list || in_list->tag < out_list->tag))
	{
	  /* This attribute only exists in ibfd. We can't merge, and we don't
	     know what the tag means, so ignore it.  */
	  err_bfd = ibfd;
	  err_tag = in_list->tag;
	  in_list = in_list->next;
	}
      else /* The tags are equal.  */
	{
	  /* As present, all attributes in the list are unknown, and
	     therefore can't be merged meaningfully.  */
	  err_bfd = obfd;
	  err_tag = out_list->tag;

	  /*  Only pass on attributes that match in both inputs.  */
	  if (in_list->attr.i != out_list->attr.i
	      || (in_list->attr.s == NULL) != (out_list->attr.s == NULL)
	      || (in_list->attr.s && out_list->attr.s
		  && strcmp (in_list->attr.s, out_list->attr.s) != 0))
	    {
	      /* No match.  Delete the attribute.  */
	      *out_listp = out_list->next;
	      out_list = *out_listp;
	    }
	  else
	    {
	      /* Matched.  Keep the attribute and move to the next.  */
	      out_list = out_list->next;
	      in_list = in_list->next;
	    }
	}

      if (err_bfd)
	result = result
	  && get_elf_backend_data (err_bfd)->obj_attrs_handle_unknown (err_bfd,
								       err_tag);
    }

  return result;
}
