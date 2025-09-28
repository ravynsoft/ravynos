/* Simple subrs.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#ifndef ENOTSUP
#define ENOTSUP ENOSYS
#endif

int _libctf_version = CTF_VERSION;	      /* Library client version.  */
int _libctf_debug = 0;			      /* Debugging messages enabled.  */

/* Private, read-only mmap from a file, with fallback to copying.

   No handling of page-offset issues at all: the caller must allow for that. */

_libctf_malloc_ void *
ctf_mmap (size_t length, size_t offset, int fd)
{
  void *data;

#ifdef HAVE_MMAP
  data = mmap (NULL, length, PROT_READ, MAP_PRIVATE, fd, offset);
  if (data == MAP_FAILED)
    data = NULL;
#else
  if ((data = malloc (length)) != NULL)
    {
      if (ctf_pread (fd, data, length, offset) <= 0)
	{
	  free (data);
	  data = NULL;
	}
    }
#endif
  return data;
}

void
ctf_munmap (void *buf, size_t length _libctf_unused_)
{
#ifdef HAVE_MMAP
  (void) munmap (buf, length);
#else
  free (buf);
#endif
}

ssize_t
ctf_pread (int fd, void *buf, ssize_t count, off_t offset)
{
  ssize_t len;
  size_t acc = 0;
  char *data = (char *) buf;

#ifdef HAVE_PREAD
  while (count > 0)
    {
      errno = 0;
      if (((len = pread (fd, data, count, offset)) < 0) &&
	  errno != EINTR)
	  return len;
      if (errno == EINTR)
	continue;

      acc += len;
      if (len == 0)				/* EOF.  */
	return acc;

      count -= len;
      offset += len;
      data += len;
    }
  return acc;
#else
  off_t orig_off;

  if ((orig_off = lseek (fd, 0, SEEK_CUR)) < 0)
    return -1;
  if ((lseek (fd, offset, SEEK_SET)) < 0)
    return -1;

  while (count > 0)
    {
      errno = 0;
      if (((len = read (fd, data, count)) < 0) &&
	  errno != EINTR)
	  return len;
      if (errno == EINTR)
	continue;

      acc += len;
      if (len == 0)				/* EOF.  */
	break;

      count -= len;
      data += len;
    }
  if ((lseek (fd, orig_off, SEEK_SET)) < 0)
    return -1;					/* offset is smashed.  */
#endif

  return acc;
}

/* Set the CTF library client version to the specified version.  If version is
   zero, we just return the default library version number.  */
int
ctf_version (int version)
{
  if (version < 0)
    {
      errno = EINVAL;
      return -1;
    }

  if (version > 0)
    {
      /*  Dynamic version switching is not presently supported. */
      if (version != CTF_VERSION)
	{
	  errno = ENOTSUP;
	  return -1;
	}
      ctf_dprintf ("ctf_version: client using version %d\n", version);
      _libctf_version = version;
    }

  return _libctf_version;
}

void
libctf_init_debug (void)
{
  static int inited;
  if (!inited)
    {
      _libctf_debug = getenv ("LIBCTF_DEBUG") != NULL;
      inited = 1;
    }
}

void ctf_setdebug (int debug)
{
  /* Ensure that libctf_init_debug() has been called, so that we don't get our
     debugging-on-or-off smashed by the next call.  */

  libctf_init_debug();
  _libctf_debug = debug;
  ctf_dprintf ("CTF debugging set to %i\n", debug);
}

int ctf_getdebug (void)
{
  return _libctf_debug;
}

_libctf_printflike_ (1, 2)
void ctf_dprintf (const char *format, ...)
{
  if (_libctf_unlikely_ (_libctf_debug))
    {
      va_list alist;

      va_start (alist, format);
      fflush (stdout);
      (void) fputs ("libctf DEBUG: ", stderr);
      (void) vfprintf (stderr, format, alist);
      va_end (alist);
    }
}

/* This needs more attention to thread-safety later on.  */
static ctf_list_t open_errors;

/* Errors and warnings.  Report the warning or error to the list in FP (or the
   open errors list if NULL): if ERR is nonzero it is the errno to report to the
   debug stream instead of that recorded on fp.  */
_libctf_printflike_ (4, 5)
extern void
ctf_err_warn (ctf_dict_t *fp, int is_warning, int err,
	      const char *format, ...)
{
  va_list alist;
  ctf_err_warning_t *cew;

  /* Don't bother reporting errors here: we can't do much about them if they
     happen.  If we're so short of memory that a tiny malloc doesn't work, a
     vfprintf isn't going to work either and the caller will have to rely on the
     ENOMEM return they'll be getting in short order anyway.  */

  if ((cew = malloc (sizeof (ctf_err_warning_t))) == NULL)
    return;

  cew->cew_is_warning = is_warning;
  va_start (alist, format);
  if (vasprintf (&cew->cew_text, format, alist) < 0)
    {
      free (cew);
      va_end (alist);
      return;
    }
  va_end (alist);

  /* Include the error code only if there is one; if this is not a warning,
     only use the error code if it was explicitly passed and is nonzero.
     (Warnings may not have a meaningful error code, since the warning may not
     lead to unwinding up to the user.)  */
  if ((!is_warning && (err != 0 || (fp && ctf_errno (fp) != 0)))
      || (is_warning && err != 0))
    ctf_dprintf ("%s: %s (%s)\n", is_warning ? _("error") : _("warning"),
		 cew->cew_text, err != 0 ? ctf_errmsg (err)
		 : ctf_errmsg (ctf_errno (fp)));
  else
    ctf_dprintf ("%s: %s\n", is_warning ? _("error") : _("warning"),
		 cew->cew_text);

  if (fp != NULL)
    ctf_list_append (&fp->ctf_errs_warnings, cew);
  else
    ctf_list_append (&open_errors, cew);
}

/* Move all the errors/warnings from an fp into the open_errors.  */
void
ctf_err_warn_to_open (ctf_dict_t *fp)
{
  ctf_list_splice (&open_errors, &fp->ctf_errs_warnings);
}

/* Error-warning reporting: an 'iterator' that returns errors and warnings from
   the error/warning list, in order of emission.  Errors and warnings are popped
   after return: the caller must free the returned error-text pointer.

   An fp of NULL returns CTF-open-time errors from the open_errors variable
   above.

   The treatment of errors from this function itself is somewhat unusual: it
   will often be called on an error path, so we don't want to overwrite the
   ctf_errno unless we have no choice.  So, like ctf_bufopen et al, this
   function takes an errp pointer where errors are reported.  The pointer is
   optional: if not set, errors are reported via the fp (if non-NULL).  Calls
   with neither fp nor errp set are mildly problematic because there is no clear
   way to report end-of-iteration: you just have to assume that a NULL return
   means the end, and not an iterator error.  */

char *
ctf_errwarning_next (ctf_dict_t *fp, ctf_next_t **it, int *is_warning,
		     int *errp)
{
  ctf_next_t *i = *it;
  char *ret;
  ctf_list_t *errlist;
  ctf_err_warning_t *cew;

  if (fp)
    errlist = &fp->ctf_errs_warnings;
  else
    errlist = &open_errors;

  if (!i)
    {
      if ((i = ctf_next_create ()) == NULL)
	{
	  if (errp)
	    *errp = ENOMEM;
	  else if (fp)
	    ctf_set_errno (fp, ENOMEM);
	  return NULL;
	}

      i->cu.ctn_fp = fp;
      i->ctn_iter_fun = (void (*) (void)) ctf_errwarning_next;
      *it = i;
    }

  if ((void (*) (void)) ctf_errwarning_next != i->ctn_iter_fun)
    {
      if (errp)
	*errp = ECTF_NEXT_WRONGFUN;
      else if (fp)
	ctf_set_errno (fp, ECTF_NEXT_WRONGFUN);
      return NULL;
    }

  if (fp != i->cu.ctn_fp)
    {
      if (errp)
	*errp = ECTF_NEXT_WRONGFP;
      else if (fp)
	ctf_set_errno (fp, ECTF_NEXT_WRONGFP);
      return NULL;
    }

  cew = ctf_list_next (errlist);

  if (!cew)
    {
      ctf_next_destroy (i);
      *it = NULL;
      if (errp)
	*errp = ECTF_NEXT_END;
      else if (fp)
	ctf_set_errno (fp, ECTF_NEXT_END);
      return NULL;
    }

  if (is_warning)
    *is_warning = cew->cew_is_warning;
  ret = cew->cew_text;
  ctf_list_delete (errlist, cew);
  free (cew);
  return ret;
}

void
ctf_assert_fail_internal (ctf_dict_t *fp, const char *file, size_t line,
			  const char *exprstr)
{
  ctf_err_warn (fp, 0, ECTF_INTERNAL, _("%s: %lu: libctf assertion failed: %s"),
		file, (long unsigned int) line, exprstr);
  ctf_set_errno (fp, ECTF_INTERNAL);
}
