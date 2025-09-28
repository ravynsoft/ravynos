/* Return the canonical absolute name of a given file.
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include "canonicalize.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <filename.h>
#include <idx.h>
#include <intprops.h>
#include <scratch_buffer.h>

#include "attribute.h"
#include "file-set.h"
#include "hash-triple.h"
#include "xalloc.h"

#ifndef DOUBLE_SLASH_IS_DISTINCT_ROOT
# define DOUBLE_SLASH_IS_DISTINCT_ROOT false
#endif

#if ISSLASH ('\\')
# define SLASHES "/\\"
#else
# define SLASHES "/"
#endif

/* Avoid false GCC warning "'end_idx' may be used uninitialized".  */
#if __GNUC__ + (__GNUC_MINOR__ >= 7) > 4
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

/* Return true if FILE's existence can be shown, false (setting errno)
   otherwise.  Follow symbolic links.  */
static bool
file_accessible (char const *file)
{
# if HAVE_FACCESSAT
  return faccessat (AT_FDCWD, file, F_OK, AT_EACCESS) == 0;
# else
  struct stat st;
  return stat (file, &st) == 0 || errno == EOVERFLOW;
# endif
}

/* True if concatenating END as a suffix to a file name means that the
   code needs to check that the file name is that of a searchable
   directory, since the canonicalize_filename_mode_stk code won't
   check this later anyway when it checks an ordinary file name
   component within END.  END must either be empty, or start with a
   slash.  */

static bool _GL_ATTRIBUTE_PURE
suffix_requires_dir_check (char const *end)
{
  /* If END does not start with a slash, the suffix is OK.  */
  while (ISSLASH (*end))
    {
      /* Two or more slashes act like a single slash.  */
      do
        end++;
      while (ISSLASH (*end));

      switch (*end++)
        {
        default: return false;  /* An ordinary file name component is OK.  */
        case '\0': return true; /* Trailing "/" is trouble.  */
        case '.': break;        /* Possibly "." or "..".  */
        }
      /* Trailing "/.", or "/.." even if not trailing, is trouble.  */
      if (!*end || (*end == '.' && (!end[1] || ISSLASH (end[1]))))
        return true;
    }

  return false;
}

/* Append this to a file name to test whether it is a searchable directory.
   On POSIX platforms "/" suffices, but "/./" is sometimes needed on
   macOS 10.13 <https://bugs.gnu.org/30350>, and should also work on
   platforms like AIX 7.2 that need at least "/.".  */

#ifdef LSTAT_FOLLOWS_SLASHED_SYMLINK
static char const dir_suffix[] = "/";
#else
static char const dir_suffix[] = "/./";
#endif

/* Return true if DIR is a searchable dir, false (setting errno) otherwise.
   DIREND points to the NUL byte at the end of the DIR string.
   Store garbage into DIREND[0 .. strlen (dir_suffix)].  */

static bool
dir_check (char *dir, char *dirend)
{
  strcpy (dirend, dir_suffix);
  return file_accessible (dir);
}

#if !((HAVE_CANONICALIZE_FILE_NAME && FUNC_REALPATH_WORKS)      \
      || GNULIB_CANONICALIZE_LGPL)
/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any ".", ".." components nor any repeated file name
   separators ('/') or symlinks.  All components must exist.
   The result is malloc'd.  */

char *
canonicalize_file_name (const char *name)
{
  return canonicalize_filename_mode (name, CAN_EXISTING);
}
#endif /* !HAVE_CANONICALIZE_FILE_NAME */

static bool
multiple_bits_set (canonicalize_mode_t i)
{
  return (i & (i - 1)) != 0;
}

/* Return true if we've already seen the triple, <FILENAME, dev, ino>.
   If *HT is not initialized, initialize it.  */
static bool
seen_triple (Hash_table **ht, char const *filename, struct stat const *st)
{
  if (*ht == NULL)
    {
      idx_t initial_capacity = 7;
      *ht = hash_initialize (initial_capacity,
                            NULL,
                            triple_hash,
                            triple_compare_ino_str,
                            triple_free);
      if (*ht == NULL)
        xalloc_die ();
    }

  if (seen_file (*ht, filename, st))
    return true;

  record_file (*ht, filename, st);
  return false;
}

/* Scratch buffers used by canonicalize_filename_mode_stk and managed
   by __realpath.  */
struct realpath_bufs
{
  struct scratch_buffer rname;
  struct scratch_buffer extra;
  struct scratch_buffer link;
};

static char *
canonicalize_filename_mode_stk (const char *name, canonicalize_mode_t can_mode,
                                struct realpath_bufs *bufs)
{
  char *dest;
  char const *start;
  char const *end;
  Hash_table *ht = NULL;
  bool logical = (can_mode & CAN_NOLINKS) != 0;
  int num_links = 0;

  canonicalize_mode_t can_exist = can_mode & CAN_MODE_MASK;
  if (multiple_bits_set (can_exist))
    {
      errno = EINVAL;
      return NULL;
    }

  if (name == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  if (name[0] == '\0')
    {
      errno = ENOENT;
      return NULL;
    }

  char *rname = bufs->rname.data;
  bool end_in_extra_buffer = false;
  bool failed = true;

  /* This is always zero for Posix hosts, but can be 2 for MS-Windows
     and MS-DOS X:/foo/bar file names.  */
  idx_t prefix_len = FILE_SYSTEM_PREFIX_LEN (name);

  if (!IS_ABSOLUTE_FILE_NAME (name))
    {
      while (!getcwd (bufs->rname.data, bufs->rname.length))
        {
          switch (errno)
            {
            case ERANGE:
              if (scratch_buffer_grow (&bufs->rname))
                break;
              FALLTHROUGH;
            case ENOMEM:
              xalloc_die ();

            default:
              dest = rname;
              goto error;
            }
          rname = bufs->rname.data;
        }
      dest = rawmemchr (rname, '\0');
      start = name;
      prefix_len = FILE_SYSTEM_PREFIX_LEN (rname);
    }
  else
    {
      dest = mempcpy (rname, name, prefix_len);
      *dest++ = '/';
      if (DOUBLE_SLASH_IS_DISTINCT_ROOT)
        {
          if (prefix_len == 0 /* implies ISSLASH (name[0]) */
              && ISSLASH (name[1]) && !ISSLASH (name[2]))
            {
              *dest++ = '/';
#if defined _WIN32 && !defined __CYGWIN__
              /* For UNC file names '\\server\path\to\file', extend the prefix
                 to include the server: '\\server\'.  */
              {
                idx_t i;
                for (i = 2; name[i] != '\0' && !ISSLASH (name[i]); )
                  i++;
                if (name[i] != '\0' /* implies ISSLASH (name[i]) */
                    && i + 1 < bufs->rname.length)
                  {
                    prefix_len = i;
                    memcpy (dest, name + 2, i - 2 + 1);
                    dest += i - 2 + 1;
                  }
                else
                  {
                    /* Either name = '\\server'; this is an invalid file name.
                       Or name = '\\server\...' and server is more than
                       bufs->rname.length - 4 bytes long.  In either
                       case, stop the UNC processing.  */
                  }
              }
#endif
            }
          *dest = '\0';
        }
      start = name + prefix_len;
    }

  for ( ; *start; start = end)
    {
      /* Skip sequence of multiple file name separators.  */
      while (ISSLASH (*start))
        ++start;

      /* Find end of component.  */
      for (end = start; *end && !ISSLASH (*end); ++end)
        /* Nothing.  */;

      /* Length of this file name component; it can be zero if a file
         name ends in '/'.  */
      idx_t startlen = end - start;

      if (startlen == 0)
        break;
      else if (startlen == 1 && start[0] == '.')
        /* nothing */;
      else if (startlen == 2 && start[0] == '.' && start[1] == '.')
        {
          /* Back up to previous component, ignore if at root already.  */
          if (dest > rname + prefix_len + 1)
            for (--dest; dest > rname && !ISSLASH (dest[-1]); --dest)
              continue;
          if (DOUBLE_SLASH_IS_DISTINCT_ROOT
              && dest == rname + 1 && !prefix_len
              && ISSLASH (*dest) && !ISSLASH (dest[1]))
            dest++;
        }
      else
        {
          if (!ISSLASH (dest[-1]))
            *dest++ = '/';

          while (rname + bufs->rname.length - dest
                 < startlen + sizeof dir_suffix)
            {
              idx_t dest_offset = dest - rname;
              if (!scratch_buffer_grow_preserve (&bufs->rname))
                xalloc_die ();
              rname = bufs->rname.data;
              dest = rname + dest_offset;
            }

          dest = mempcpy (dest, start, startlen);
          *dest = '\0';

          char *buf;
          ssize_t n = -1;
          if (!logical)
            {
              while (true)
                {
                  buf = bufs->link.data;
                  idx_t bufsize = bufs->link.length;
                  n = readlink (rname, buf, bufsize - 1);
                  if (n < bufsize - 1)
                    break;
                  if (!scratch_buffer_grow (&bufs->link))
                    xalloc_die ();
                }
            }
          if (0 <= n)
            {
              /* A physical traversal and RNAME is a symbolic link.  */

              if (num_links < 20)
                num_links++;
              else if (*start)
                {
                  /* Enough symlinks have been seen that it is time to
                     worry about being in a symlink cycle.
                     Get the device and inode of the parent directory, as
                     pre-2017 POSIX says this info is not reliable for
                     symlinks.  */
                  struct stat st;
                  dest[- startlen] = '\0';
                  if (stat (*rname ? rname : ".", &st) != 0)
                    goto error;
                  dest[- startlen] = *start;

                  /* Detect loops.  We cannot use the cycle-check module here,
                     since it's possible to encounter the same parent
                     directory more than once in a given traversal.  However,
                     encountering the same (parentdir, START) pair twice does
                     indicate a loop.  */
                  if (seen_triple (&ht, start, &st))
                    {
                      if (can_exist == CAN_MISSING)
                        continue;
                      errno = ELOOP;
                      goto error;
                    }
                }

              buf[n] = '\0';

              char *extra_buf = bufs->extra.data;
              idx_t end_idx;
              if (end_in_extra_buffer)
                end_idx = end - extra_buf;
              size_t len = strlen (end);
              if (INT_ADD_OVERFLOW (len, n))
                xalloc_die ();
              while (bufs->extra.length <= len + n)
                {
                  if (!scratch_buffer_grow_preserve (&bufs->extra))
                    xalloc_die ();
                  extra_buf = bufs->extra.data;
                }
              if (end_in_extra_buffer)
                end = extra_buf + end_idx;

              /* Careful here, end may be a pointer into extra_buf... */
              memmove (&extra_buf[n], end, len + 1);
              name = end = memcpy (extra_buf, buf, n);
              end_in_extra_buffer = true;

              if (IS_ABSOLUTE_FILE_NAME (buf))
                {
                  idx_t pfxlen = FILE_SYSTEM_PREFIX_LEN (buf);

                  dest = mempcpy (rname, buf, pfxlen);
                  *dest++ = '/'; /* It's an absolute symlink */
                  if (DOUBLE_SLASH_IS_DISTINCT_ROOT)
                    {
                      if (ISSLASH (buf[1]) && !ISSLASH (buf[2]) && !pfxlen)
                        *dest++ = '/';
                      *dest = '\0';
                    }
                  /* Install the new prefix to be in effect hereafter.  */
                  prefix_len = pfxlen;
                }
              else
                {
                  /* Back up to previous component, ignore if at root
                     already: */
                  if (dest > rname + prefix_len + 1)
                    for (--dest; dest > rname && !ISSLASH (dest[-1]); --dest)
                      continue;
                  if (DOUBLE_SLASH_IS_DISTINCT_ROOT && dest == rname + 1
                      && ISSLASH (*dest) && !ISSLASH (dest[1]) && !prefix_len)
                    dest++;
                }
            }
          else if (! (can_exist == CAN_MISSING
                      || (suffix_requires_dir_check (end)
                          ? dir_check (rname, dest)
                          : !logical
                          ? errno == EINVAL
                          : *end || file_accessible (rname))
                      || (can_exist == CAN_ALL_BUT_LAST
                          && errno == ENOENT
                          && !end[strspn (end, SLASHES)])))
            goto error;
        }
    }
  if (dest > rname + prefix_len + 1 && ISSLASH (dest[-1]))
    --dest;
  if (DOUBLE_SLASH_IS_DISTINCT_ROOT && dest == rname + 1 && !prefix_len
      && ISSLASH (*dest) && !ISSLASH (dest[1]))
    dest++;
  failed = false;

error:
  if (ht)
    hash_free (ht);

  if (failed)
    return NULL;

  *dest++ = '\0';
  char *result = malloc (dest - rname);
  if (!result)
    xalloc_die ();
  return memcpy (result, rname, dest - rname);
}

/* Return the canonical absolute name of file NAME, while treating
   missing elements according to CAN_MODE.  A canonical name
   does not contain any ".", ".." components nor any repeated file name
   separators ('/') or, depending on other CAN_MODE flags, symlinks.
   Whether components must exist or not depends on canonicalize mode.
   The result is malloc'd.  */

char *
canonicalize_filename_mode (const char *name, canonicalize_mode_t can_mode)
{
  struct realpath_bufs bufs;
  scratch_buffer_init (&bufs.rname);
  scratch_buffer_init (&bufs.extra);
  scratch_buffer_init (&bufs.link);
  char *result = canonicalize_filename_mode_stk (name, can_mode, &bufs);
  scratch_buffer_free (&bufs.link);
  scratch_buffer_free (&bufs.extra);
  scratch_buffer_free (&bufs.rname);
  return result;
}
