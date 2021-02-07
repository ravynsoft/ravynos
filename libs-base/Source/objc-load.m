/* objc-load - Dynamically load in Obj-C modules (Classes, Categories)

   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.

   Written by:  Adam Fedor, Pedja Bogdanovich

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

/* PS: Unloading modules is not implemented.  */

#import "common.h"
#include <stdio.h>

#if defined(NeXT_RUNTIME)
# include <objc/objc-load.h>
#endif

#ifdef __GNUSTEP_RUNTIME__
# include <objc/hooks.h>
#endif

#if defined(__CYGWIN__)
# include <windows.h>
#endif

#include "objc-load.h"
#import "Foundation/NSException.h"

#import "GSPrivate.h"

/* include the interface to the dynamic linker */
#include "dynamic-load.h"

/* dynamic_loaded is YES if the dynamic loader was sucessfully initialized. */
static BOOL	dynamic_loaded;

/* Our current callback function */
static void (*_objc_load_load_callback)(Class, struct objc_category *) = 0;

/* Check to see if there are any undefined symbols. Print them out.
*/
static int
objc_check_undefineds(FILE *errorStream)
{
  int count = __objc_dynamic_undefined_symbol_count();

  if (count != 0)
    {
      int  i;
      char **undefs;

      undefs = __objc_dynamic_list_undefined_symbols();
      if (errorStream)
	{
	  fprintf(errorStream, "Undefined symbols:\n");
	}
      for (i = 0; i < count; i++)
	{
	  if (errorStream)
	    {
	      fprintf(errorStream, "  %s\n", undefs[i]);
	    }
	}
      return 1;
    }
  return 0;
}

/* Initialize for dynamic loading */
static int
objc_initialize_loading(FILE *errorStream)
{
  NSString	*path;
#if     defined(_WIN32) || defined(__CYGWIN__)
  const unichar *fsPath;
#else  
  const char *fsPath;
#endif  

  dynamic_loaded = NO;
  path = GSPrivateExecutablePath();

  NSDebugFLLog(@"NSBundle",
    @"Debug (objc-load): initializing dynamic loader for %@", path);

  fsPath = [[path stringByDeletingLastPathComponent] fileSystemRepresentation];

  if (__objc_dynamic_init(fsPath))
    {
      if (errorStream)
	{
	  __objc_dynamic_error(errorStream,
           "Error (objc-load): Cannot initialize dynamic linker");
	}
      return 1;
    }
  else
    {
      dynamic_loaded = YES;
    }

  return 0;
}

/* A callback received from the Object initializer (_objc_exec_class).
   Do what we need to do and call our own callback.
*/
static void
objc_load_callback(Class class, struct objc_category * category)
{
  if (_objc_load_load_callback)
    {
      _objc_load_load_callback(class, category);
    }
}

#if	defined(_WIN32) || defined(__CYGWIN__)
#define	FSCHAR	unichar
#else
#define	FSCHAR	char
#endif

long
GSPrivateLoadModule(NSString *filename, FILE *errorStream,
  void (*loadCallback)(Class, struct objc_category *),
  void **header, NSString *debugFilename)
{
#ifdef NeXT_RUNTIME
  int errcode;
  dynamic_loaded = YES;
  return objc_loadModule([filename fileSystemRepresentation],
    loadCallback, &errcode);
#else
  dl_handle_t handle;
  void __objc_resolve_class_links(void);
#if !defined(__ELF__) && !defined(CON_AUTOLOAD)
  typedef void (*void_fn)();
  void_fn *ctor_list;
  int i;
#endif

  if (!dynamic_loaded)
    {
      if (objc_initialize_loading(errorStream))
	{
	  return 1;
	}
    }

  _objc_load_load_callback = loadCallback;
  _objc_load_callback = objc_load_callback;

  /* Link in the object file */
  NSDebugFLLog(@"NSBundle", @"Debug (objc-load): Linking file %@\n", filename);
  handle = __objc_dynamic_link((FSCHAR*)[filename fileSystemRepresentation],
    1, (FSCHAR*)[debugFilename fileSystemRepresentation]);
  if (handle == 0)
    {
      if (errorStream)
	{
	  __objc_dynamic_error(errorStream, "Error (objc-load)");
	}
      _objc_load_load_callback = 0;
      _objc_load_callback = 0;
      return 1;
    }

  /* If there are any undefined symbols, we can't load the bundle */
  if (objc_check_undefineds(errorStream))
    {
      __objc_dynamic_unlink(handle);
      _objc_load_load_callback = 0;
      _objc_load_callback = 0;
      return 1;
    }

#if !defined(__ELF__) && !defined(CON_AUTOLOAD)
  /* Get the constructor list and load in the objects */
  ctor_list = (void_fn *)__objc_dynamic_find_symbol(handle, CTOR_LIST);
  if (!ctor_list)
    {
      if (errorStream)
	{
	  fprintf(errorStream,
	    "Error (objc-load): Cannot load objects (no CTOR list)\n");
	}
      _objc_load_load_callback = 0;
      _objc_load_callback = 0;
      return 1;
    }

  NSDebugFLLog(@"NSBundle",
    @"Debug (objc-load): %d modules\n", (int)ctor_list[0]);
  for (i = 1; ctor_list[i]; i++)
    {
      NSDebugFLLog(@"NSBundle",
	@"Debug (objc-load): Invoking CTOR %p\n", ctor_list[i]);
      ctor_list[i]();
    }
#endif /* not __ELF__ */

#if !defined(__GNUSTEP_RUNTIME__) && !defined(__GNU_LIBOBJC__)
  __objc_resolve_class_links(); /* fill in subclass_list and sibling_class */
#endif
  _objc_load_callback = 0;
  _objc_load_load_callback = 0;
  return 0;
#endif /* not NeXT_RUNTIME */
}

long
GSPrivateUnloadModule(FILE *errorStream,
  void (*unloadCallback)(Class, struct objc_category *))
{
  if (!dynamic_loaded)
    {
      return 1;
    }

  if (errorStream)
    {
      fprintf(errorStream, "Warning: unloading modules not implemented\n");
    }
  return 0;
}


#if defined(_WIN32) || defined(__CYGWIN__)
// FIXME: We can probably get rid of this now - MinGW should include a working
// dladdr() wrapping this function, so we no longer need a Windows-only code
// path
NSString *
GSPrivateSymbolPath(Class theClass)
{
  unichar buf[MAX_PATH];
  NSString *s = nil;
  MEMORY_BASIC_INFORMATION memInfo;

  VirtualQueryEx(GetCurrentProcess(), theClass, &memInfo, sizeof(memInfo));
  if (GetModuleFileNameW(memInfo.AllocationBase, buf, sizeof(buf)))
    {
#ifdef __CYGWIN__
#warning Under Cygwin, we may want to use cygwin_conv_path() to get the unix path back?
#endif
      s = [NSString stringWithCharacters: buf length: wcslen(buf)];
    }
  return s;
}
#else
NSString *GSPrivateSymbolPath(Class theClass)
{
#if LINKER_GETSYMBOL 
  Dl_info info;

  /* This is correct: dladdr() does the opposite thing to all other UNIX
   * functions.
   * On success, return the results, otherwise fall back to use the
   * __objc_dynamic_get_symbol_path() function.
   */
  if (0 != dladdr((void*)theClass, &info))
    {
      return [NSString stringWithUTF8String: info.dli_fname];
    }
#endif

  if (theClass != nil)
    {
      const char        *prefix
#if __OBJC_GNUSTEP_RUNTIME_ABI__ >= 20
        = "._OBJC_CLASS_";
#else
        = "__objc_class_name_";
#endif
      const char        *ret;
      char              buf[125];
      char              *p = buf;
      const char        *className = class_getName(theClass);
      int               len = strlen(className);
      int               plen = strlen(prefix);

      if (len + plen + 1 > sizeof(buf))
        {
          p = malloc(len + plen + 1);

          if (p == NULL)
            {
              fprintf(stderr, "Unable to allocate memory !!");
              return nil;
            }
        }

      memcpy(p, prefix, plen);
      memcpy(&p[plen], className, len + 1);

      ret = __objc_dynamic_get_symbol_path(0, p);

      if (p != buf)
        {
          free(p);
        }

      if (ret)
        {
          return [NSString stringWithUTF8String: ret];
        }
    }
  return nil;
}
#endif
