/* BFD support for the ARM processor
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Richard Earnshaw (rwe@pegasus.esprit.ec.org)

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
#include "libbfd.h"
#include "libiberty.h"
#include "cpu-arm.h"

/* This routine is provided two arch_infos and works out which ARM
   machine which would be compatible with both and returns a pointer
   to its info structure.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
      return NULL;

  /* If a & b are for the same machine then all is well.  */
  if (a->mach == b->mach)
    return a;

  /* Otherwise if either a or b is the 'default' machine
     then it can be polymorphed into the other.  */
  if (a->the_default)
    return b;

  if (b->the_default)
    return a;

  /* So far all newer ARM architecture cores are
     supersets of previous cores.  */
  if (a->mach < b->mach)
    return b;
  else if (a->mach > b->mach)
    return a;

  /* Never reached!  */
  return NULL;
}

static struct
{
  unsigned int mach;
  char *       name;
}
processors[] =
{
  { bfd_mach_arm_2,	  "arm2"	    },
  { bfd_mach_arm_2a,	  "arm250"	    },
  { bfd_mach_arm_2a,	  "arm3"	    },
  { bfd_mach_arm_3,	  "arm6"	    },
  { bfd_mach_arm_3,	  "arm60"	    },
  { bfd_mach_arm_3,	  "arm600"	    },
  { bfd_mach_arm_3,	  "arm610"	    },
  { bfd_mach_arm_3,	  "arm620"	    },
  { bfd_mach_arm_3,	  "arm7"	    },
  { bfd_mach_arm_3,	  "arm70"	    },
  { bfd_mach_arm_3,	  "arm700"	    },
  { bfd_mach_arm_3,	  "arm700i"	    },
  { bfd_mach_arm_3,	  "arm710"	    },
  { bfd_mach_arm_3,	  "arm7100"	    },
  { bfd_mach_arm_3,	  "arm710c"	    },
  { bfd_mach_arm_4T,	  "arm710t"	    },
  { bfd_mach_arm_3,	  "arm720"	    },
  { bfd_mach_arm_4T,	  "arm720t"	    },
  { bfd_mach_arm_4T,	  "arm740t"	    },
  { bfd_mach_arm_3,	  "arm7500"	    },
  { bfd_mach_arm_3,	  "arm7500fe"	    },
  { bfd_mach_arm_3,	  "arm7d"	    },
  { bfd_mach_arm_3,	  "arm7di"	    },
  { bfd_mach_arm_3M,	  "arm7dm"	    },
  { bfd_mach_arm_3M,	  "arm7dmi"	    },
  { bfd_mach_arm_4T,	  "arm7t"	    },
  { bfd_mach_arm_4T,	  "arm7tdmi"	    },
  { bfd_mach_arm_4T,	  "arm7tdmi-s"	    },
  { bfd_mach_arm_3M,	  "arm7m"	    },
  { bfd_mach_arm_4,	  "arm8"	    },
  { bfd_mach_arm_4,	  "arm810"	    },
  { bfd_mach_arm_4,	  "arm9"	    },
  { bfd_mach_arm_4T,	  "arm920"	    },
  { bfd_mach_arm_4T,	  "arm920t"	    },
  { bfd_mach_arm_4T,	  "arm922t"	    },
  { bfd_mach_arm_5TEJ,	  "arm926ej"	    },
  { bfd_mach_arm_5TEJ,	  "arm926ejs"	    },
  { bfd_mach_arm_5TEJ,	  "arm926ej-s"	    },
  { bfd_mach_arm_4T,	  "arm940t"	    },
  { bfd_mach_arm_5TE,	  "arm946e"	    },
  { bfd_mach_arm_5TE,	  "arm946e-r0"	    },
  { bfd_mach_arm_5TE,	  "arm946e-s"	    },
  { bfd_mach_arm_5TE,	  "arm966e"	    },
  { bfd_mach_arm_5TE,	  "arm966e-r0"	    },
  { bfd_mach_arm_5TE,	  "arm966e-s"	    },
  { bfd_mach_arm_5TE,	  "arm968e-s"	    },
  { bfd_mach_arm_5TE,	  "arm9e"	    },
  { bfd_mach_arm_5TE,	  "arm9e-r0"	    },
  { bfd_mach_arm_4T,	  "arm9tdmi"	    },
  { bfd_mach_arm_5TE,	  "arm1020"	    },
  { bfd_mach_arm_5T,	  "arm1020t"	    },
  { bfd_mach_arm_5TE,	  "arm1020e"	    },
  { bfd_mach_arm_5TE,	  "arm1022e"	    },
  { bfd_mach_arm_5TEJ,	  "arm1026ejs"	    },
  { bfd_mach_arm_5TEJ,	  "arm1026ej-s"	    },
  { bfd_mach_arm_5TE,	  "arm10e"	    },
  { bfd_mach_arm_5T,	  "arm10t"	    },
  { bfd_mach_arm_5T,	  "arm10tdmi"	    },
  { bfd_mach_arm_6,	  "arm1136j-s"	    },
  { bfd_mach_arm_6,	  "arm1136js"	    },
  { bfd_mach_arm_6,	  "arm1136jf-s"	    },
  { bfd_mach_arm_6,	  "arm1136jfs"	    },
  { bfd_mach_arm_6KZ,	  "arm1176jz-s"	    },
  { bfd_mach_arm_6KZ,	  "arm1176jzf-s"    },
  { bfd_mach_arm_6T2,	  "arm1156t2-s"	    },
  { bfd_mach_arm_6T2,	  "arm1156t2f-s"    },
  { bfd_mach_arm_7,	  "cortex-a5"	    },
  { bfd_mach_arm_7,	  "cortex-a7"	    },
  { bfd_mach_arm_7,	  "cortex-a8"	    },
  { bfd_mach_arm_7,	  "cortex-a9"	    },
  { bfd_mach_arm_7,	  "cortex-a12"	    },
  { bfd_mach_arm_7,	  "cortex-a15"	    },
  { bfd_mach_arm_7,	  "cortex-a17"	    },
  { bfd_mach_arm_8,	  "cortex-a32"	    },
  { bfd_mach_arm_8,	  "cortex-a35"	    },
  { bfd_mach_arm_8,	  "cortex-a53"	    },
  { bfd_mach_arm_8,	  "cortex-a55"	    },
  { bfd_mach_arm_8,	  "cortex-a57"	    },
  { bfd_mach_arm_8,	  "cortex-a72"	    },
  { bfd_mach_arm_8,	  "cortex-a73"	    },
  { bfd_mach_arm_8,	  "cortex-a75"	    },
  { bfd_mach_arm_8,	  "cortex-a76"	    },
  { bfd_mach_arm_8,	  "cortex-a76ae"    },
  { bfd_mach_arm_8,	  "cortex-a77"	    },
  { bfd_mach_arm_8,	  "cortex-a78"	    },
  { bfd_mach_arm_8,	  "cortex-a78ae"    },
  { bfd_mach_arm_8,	  "cortex-a78c"     },
  { bfd_mach_arm_6SM,	  "cortex-m0"	    },
  { bfd_mach_arm_6SM,	  "cortex-m0plus"   },
  { bfd_mach_arm_6SM,	  "cortex-m1"	    },
  { bfd_mach_arm_8M_BASE, "cortex-m23"	    },
  { bfd_mach_arm_7,	  "cortex-m3"	    },
  { bfd_mach_arm_8M_MAIN, "cortex-m33"	    },
  { bfd_mach_arm_8M_MAIN, "cortex-m35p"	    },
  { bfd_mach_arm_7EM,	  "cortex-m4"	    },
  { bfd_mach_arm_7EM,	  "cortex-m7"	    },
  { bfd_mach_arm_7,	  "cortex-r4"	    },
  { bfd_mach_arm_7,	  "cortex-r4f"	    },
  { bfd_mach_arm_7,	  "cortex-r5"	    },
  { bfd_mach_arm_8R,	  "cortex-r52"	    },
  { bfd_mach_arm_8R,	  "cortex-r52plus"	    },
  { bfd_mach_arm_7,	  "cortex-r7"	    },
  { bfd_mach_arm_7,	  "cortex-r8"	    },
  { bfd_mach_arm_8,	  "cortex-x1"	    },
  { bfd_mach_arm_8,	  "cortex-x1c"	    },
  { bfd_mach_arm_4T,	  "ep9312"	    },
  { bfd_mach_arm_8,	  "exynos-m1"	    },
  { bfd_mach_arm_4,	  "fa526"	    },
  { bfd_mach_arm_5TE,	  "fa606te"	    },
  { bfd_mach_arm_5TE,	  "fa616te"	    },
  { bfd_mach_arm_4,	  "fa626"	    },
  { bfd_mach_arm_5TE,	  "fa626te"	    },
  { bfd_mach_arm_5TE,	  "fa726te"	    },
  { bfd_mach_arm_5TE,	  "fmp626"	    },
  { bfd_mach_arm_XScale,  "i80200"	    },
  { bfd_mach_arm_7,	  "marvell-pj4"	    },
  { bfd_mach_arm_7,	  "marvell-whitney" },
  { bfd_mach_arm_6K,	  "mpcore"	    },
  { bfd_mach_arm_6K,	  "mpcorenovfp"	    },
  { bfd_mach_arm_4,	  "sa1"		    },
  { bfd_mach_arm_4,	  "strongarm"	    },
  { bfd_mach_arm_4,	  "strongarm1"	    },
  { bfd_mach_arm_4,	  "strongarm110"    },
  { bfd_mach_arm_4,	  "strongarm1100"   },
  { bfd_mach_arm_4,	  "strongarm1110"   },
  { bfd_mach_arm_XScale,  "xscale"	    },
  { bfd_mach_arm_8,	  "xgene1"	    },
  { bfd_mach_arm_8,	  "xgene2"	    },
  { bfd_mach_arm_9,	  "cortex-a710"	    },
  { bfd_mach_arm_ep9312,  "ep9312"	    },
  { bfd_mach_arm_iWMMXt,  "iwmmxt"	    },
  { bfd_mach_arm_iWMMXt2, "iwmmxt2"	    },
  { bfd_mach_arm_unknown, "arm_any"	    }
};

static bool
scan (const struct bfd_arch_info *info, const char *string)
{
  int  i;

  /* First test for an exact match.  */
  if (strcasecmp (string, info->printable_name) == 0)
    return true;

  /* If there is a prefix of "arm:" then skip it.  */
  const char * colon;
  if ((colon = strchr (string, ':')) != NULL)
    {
      if (strncasecmp (string, "arm", colon - string) != 0)
	return false;
      string = colon + 1;
    }

  /* Next check for a processor name instead of an Architecture name.  */
  for (i = sizeof (processors) / sizeof (processors[0]); i--;)
    {
      if (strcasecmp (string, processors [i].name) == 0)
	break;
    }

  if (i != -1 && info->mach == processors [i].mach)
    return true;

  /* Finally check for the default architecture.  */
  if (strcasecmp (string, "arm") == 0)
    return info->the_default;

  return false;
}

#define N(number, print, default, next)  \
{  32, 32, 8, bfd_arch_arm, number, "arm", print, 4, default, compatible, \
    scan, bfd_arch_default_fill, next, 0 }

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_arm_2,         "armv2",          false, & arch_info_struct[1]),
  N (bfd_mach_arm_2a,        "armv2a",         false, & arch_info_struct[2]),
  N (bfd_mach_arm_3,         "armv3",          false, & arch_info_struct[3]),
  N (bfd_mach_arm_3M,        "armv3m",         false, & arch_info_struct[4]),
  N (bfd_mach_arm_4,         "armv4",          false, & arch_info_struct[5]),
  N (bfd_mach_arm_4T,        "armv4t",         false, & arch_info_struct[6]),
  N (bfd_mach_arm_5,         "armv5",          false, & arch_info_struct[7]),
  N (bfd_mach_arm_5T,        "armv5t",         false, & arch_info_struct[8]),
  N (bfd_mach_arm_5TE,       "armv5te",        false, & arch_info_struct[9]),
  N (bfd_mach_arm_XScale,    "xscale",         false, & arch_info_struct[10]),
  N (bfd_mach_arm_ep9312,    "ep9312",         false, & arch_info_struct[11]),
  N (bfd_mach_arm_iWMMXt,    "iwmmxt",         false, & arch_info_struct[12]),
  N (bfd_mach_arm_iWMMXt2,   "iwmmxt2",        false, & arch_info_struct[13]),
  N (bfd_mach_arm_5TEJ,      "armv5tej",       false, & arch_info_struct[14]),
  N (bfd_mach_arm_6,         "armv6",          false, & arch_info_struct[15]),
  N (bfd_mach_arm_6KZ,       "armv6kz",        false, & arch_info_struct[16]),
  N (bfd_mach_arm_6T2,       "armv6t2",        false, & arch_info_struct[17]),
  N (bfd_mach_arm_6K,        "armv6k",         false, & arch_info_struct[18]),
  N (bfd_mach_arm_7,         "armv7",          false, & arch_info_struct[19]),
  N (bfd_mach_arm_6M,        "armv6-m",        false, & arch_info_struct[20]),
  N (bfd_mach_arm_6SM,       "armv6s-m",       false, & arch_info_struct[21]),
  N (bfd_mach_arm_7EM,       "armv7e-m",       false, & arch_info_struct[22]),
  N (bfd_mach_arm_8,         "armv8-a",        false, & arch_info_struct[23]),
  N (bfd_mach_arm_8R,        "armv8-r",        false, & arch_info_struct[24]),
  N (bfd_mach_arm_8M_BASE,   "armv8-m.base",   false, & arch_info_struct[25]),
  N (bfd_mach_arm_8M_MAIN,   "armv8-m.main",   false, & arch_info_struct[26]),
  N (bfd_mach_arm_8_1M_MAIN, "armv8.1-m.main", false, & arch_info_struct[27]),
  N (bfd_mach_arm_9,         "armv9-a",        false, & arch_info_struct[28]),
  N (bfd_mach_arm_unknown,   "arm_any",        false, NULL)
};

const bfd_arch_info_type bfd_arm_arch =
  N (0, "arm", true, & arch_info_struct[0]);

/* Support functions used by both the COFF and ELF versions of the ARM port.  */

/* Handle the merging of the 'machine' settings of input file IBFD
   and an output file OBFD.  These values actually represent the
   different possible ARM architecture variants.
   Returns TRUE if they were merged successfully or FALSE otherwise.  */

bool
bfd_arm_merge_machines (bfd *ibfd, bfd *obfd)
{
  unsigned int in  = bfd_get_mach (ibfd);
  unsigned int out = bfd_get_mach (obfd);

  /* If the output architecture is unknown, we now have a value to set.  */
  if (out == bfd_mach_arm_unknown)
    bfd_set_arch_mach (obfd, bfd_arch_arm, in);

  /* If the input architecture is unknown,
     then so must be the output architecture.  */
  else if (in == bfd_mach_arm_unknown)
    /* FIXME: We ought to have some way to
       override this on the command line.  */
    bfd_set_arch_mach (obfd, bfd_arch_arm, bfd_mach_arm_unknown);

  /* If they are the same then nothing needs to be done.  */
  else if (out == in)
    ;

  /* Otherwise the general principle that a earlier architecture can be
     linked with a later architecture to produce a binary that will execute
     on the later architecture.

     We fail however if we attempt to link a Cirrus EP9312 binary with an
     Intel XScale binary, since these architecture have co-processors which
     will not both be present on the same physical hardware.  */
  else if (in == bfd_mach_arm_ep9312
	   && (out == bfd_mach_arm_XScale
	       || out == bfd_mach_arm_iWMMXt
	       || out == bfd_mach_arm_iWMMXt2))
    {
      /* xgettext: c-format */
      _bfd_error_handler (_("error: %pB is compiled for the EP9312, "
			    "whereas %pB is compiled for XScale"),
			  ibfd, obfd);
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
  else if (out == bfd_mach_arm_ep9312
	   && (in == bfd_mach_arm_XScale
	       || in == bfd_mach_arm_iWMMXt
	       || in == bfd_mach_arm_iWMMXt2))
    {
      /* xgettext: c-format */
      _bfd_error_handler (_("error: %pB is compiled for the EP9312, "
			    "whereas %pB is compiled for XScale"),
			  obfd, ibfd);
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }
  else if (in > out)
    bfd_set_arch_mach (obfd, bfd_arch_arm, in);
  /* else
     Nothing to do.  */

  return true;
}

typedef struct
{
  unsigned char	namesz[4];	/* Size of entry's owner string.  */
  unsigned char	descsz[4];	/* Size of the note descriptor.  */
  unsigned char	type[4];	/* Interpretation of the descriptor.  */
  char		name[1];	/* Start of the name+desc data.  */
} arm_Note;

static bool
arm_check_note (bfd *abfd,
		bfd_byte *buffer,
		bfd_size_type buffer_size,
		const char *expected_name,
		char **description_return)
{
  unsigned long namesz;
  unsigned long descsz;
  unsigned long type;
  char *	descr;

  if (buffer_size < offsetof (arm_Note, name))
    return false;

  /* We have to extract the values this way to allow for a
     host whose endian-ness is different from the target.  */
  namesz = bfd_get_32 (abfd, buffer);
  descsz = bfd_get_32 (abfd, buffer + offsetof (arm_Note, descsz));
  type   = bfd_get_32 (abfd, buffer + offsetof (arm_Note, type));
  descr  = (char *) buffer + offsetof (arm_Note, name);

  /* Check for buffer overflow.  */
  if (namesz + descsz + offsetof (arm_Note, name) > buffer_size)
    return false;

  if (expected_name == NULL)
    {
      if (namesz != 0)
	return false;
    }
  else
    {
      if (namesz != ((strlen (expected_name) + 1 + 3) & ~3))
	return false;

      if (strcmp (descr, expected_name) != 0)
	return false;

      descr += (namesz + 3) & ~3;
    }

  /* FIXME: We should probably check the type as well.  */
  (void) type;

  if (description_return != NULL)
    * description_return = descr;

  return true;
}

#define NOTE_ARCH_STRING	"arch: "

bool
bfd_arm_update_notes (bfd *abfd, const char *note_section)
{
  asection *	 arm_arch_section;
  bfd_size_type	 buffer_size;
  bfd_byte *	 buffer;
  char *	 arch_string;
  char *	 expected;

  /* Look for a note section.  If one is present check the architecture
     string encoded in it, and set it to the current architecture if it is
     different.  */
  arm_arch_section = bfd_get_section_by_name (abfd, note_section);

  if (arm_arch_section == NULL
      || (arm_arch_section->flags & SEC_HAS_CONTENTS) == 0)
    return true;

  buffer_size = arm_arch_section->size;
  if (buffer_size == 0)
    return false;

  if (!bfd_malloc_and_get_section (abfd, arm_arch_section, &buffer))
    goto FAIL;

  /* Parse the note.  */
  if (! arm_check_note (abfd, buffer, buffer_size, NOTE_ARCH_STRING, & arch_string))
    goto FAIL;

  /* Check the architecture in the note against the architecture of the bfd.
     Newer architectures versions should not be added here as build attribute
     are a better mechanism to convey ISA used.  */
  switch (bfd_get_mach (abfd))
    {
    default:
    case bfd_mach_arm_unknown: expected = "unknown"; break;
    case bfd_mach_arm_2:       expected = "armv2"; break;
    case bfd_mach_arm_2a:      expected = "armv2a"; break;
    case bfd_mach_arm_3:       expected = "armv3"; break;
    case bfd_mach_arm_3M:      expected = "armv3M"; break;
    case bfd_mach_arm_4:       expected = "armv4"; break;
    case bfd_mach_arm_4T:      expected = "armv4t"; break;
    case bfd_mach_arm_5:       expected = "armv5"; break;
    case bfd_mach_arm_5T:      expected = "armv5t"; break;
    case bfd_mach_arm_5TE:     expected = "armv5te"; break;
    case bfd_mach_arm_XScale:  expected = "XScale"; break;
    case bfd_mach_arm_ep9312:  expected = "ep9312"; break;
    case bfd_mach_arm_iWMMXt:  expected = "iWMMXt"; break;
    case bfd_mach_arm_iWMMXt2: expected = "iWMMXt2"; break;
    }

  if (strcmp (arch_string, expected) != 0)
    {
      strcpy ((char *) buffer + (offsetof (arm_Note, name)
				 + ((strlen (NOTE_ARCH_STRING) + 3) & ~3)),
	      expected);

      if (! bfd_set_section_contents (abfd, arm_arch_section, buffer,
				      (file_ptr) 0, buffer_size))
	{
	  _bfd_error_handler
	    /* xgettext: c-format */
	    (_("warning: unable to update contents of %s section in %pB"),
	     note_section, abfd);
	  goto FAIL;
	}
    }

  free (buffer);
  return true;

 FAIL:
  free (buffer);
  return false;
}


static struct
{
  const char * string;
  unsigned int mach;
}

/* Newer architectures versions should not be added here as build attribute are
   a better mechanism to convey ISA used.  */
architectures[] =
{
  { "armv2",   bfd_mach_arm_2 },
  { "armv2a",  bfd_mach_arm_2a },
  { "armv3",   bfd_mach_arm_3 },
  { "armv3M",  bfd_mach_arm_3M },
  { "armv4",   bfd_mach_arm_4 },
  { "armv4t",  bfd_mach_arm_4T },
  { "armv5",   bfd_mach_arm_5 },
  { "armv5t",  bfd_mach_arm_5T },
  { "armv5te", bfd_mach_arm_5TE },
  { "XScale",  bfd_mach_arm_XScale },
  { "ep9312",  bfd_mach_arm_ep9312 },
  { "iWMMXt",  bfd_mach_arm_iWMMXt },
  { "iWMMXt2", bfd_mach_arm_iWMMXt2 },
  { "arm_any", bfd_mach_arm_unknown }
};

/* Extract the machine number stored in a note section.  */
unsigned int
bfd_arm_get_mach_from_notes (bfd *abfd, const char *note_section)
{
  asection *	 arm_arch_section;
  bfd_size_type	 buffer_size;
  bfd_byte *	 buffer;
  char *	 arch_string;
  int		 i;

  /* Look for a note section.  If one is present check the architecture
     string encoded in it, and set it to the current architecture if it is
     different.  */
  arm_arch_section = bfd_get_section_by_name (abfd, note_section);

  if (arm_arch_section == NULL
      || (arm_arch_section->flags & SEC_HAS_CONTENTS) == 0)
    return bfd_mach_arm_unknown;

  buffer_size = arm_arch_section->size;
  if (buffer_size == 0)
    return bfd_mach_arm_unknown;

  if (!bfd_malloc_and_get_section (abfd, arm_arch_section, &buffer))
    goto FAIL;

  /* Parse the note.  */
  if (! arm_check_note (abfd, buffer, buffer_size, NOTE_ARCH_STRING, & arch_string))
    goto FAIL;

  /* Interpret the architecture string.  */
  for (i = ARRAY_SIZE (architectures); i--;)
    if (strcmp (arch_string, architectures[i].string) == 0)
      {
	free (buffer);
	return architectures[i].mach;
      }

 FAIL:
  free (buffer);
  return bfd_mach_arm_unknown;
}

bool
bfd_is_arm_special_symbol_name (const char * name, int type)
{
  /* The ARM compiler outputs several obsolete forms.  Recognize them
     in addition to the standard $a, $t and $d.  We are somewhat loose
     in what we accept here, since the full set is not documented.  */
  if (!name || name[0] != '$')
    return false;
  if (name[1] == 'a' || name[1] == 't' || name[1] == 'd')
    type &= BFD_ARM_SPECIAL_SYM_TYPE_MAP;
  else if (name[1] == 'm' || name[1] == 'f' || name[1] == 'p')
    type &= BFD_ARM_SPECIAL_SYM_TYPE_TAG;
  else if (name[1] >= 'a' && name[1] <= 'z')
    type &= BFD_ARM_SPECIAL_SYM_TYPE_OTHER;
  else
    return false;

  return (type != 0 && (name[2] == 0 || name[2] == '.'));
}

