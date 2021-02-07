/*
    simple-load - Definitions and translations for dynamic loading with 
	the simple dynamic liading library (dl).

    Copyright (C) 1995, Free Software Foundation

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more details. 
        
You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.

    BUGS:
	- In SunOS 4.1, dlopen will only resolve references into the main
	module and not into other modules loaded earlier. dlopen will exit
	if there are undefined symbols. Later versions (e.g. 5.3) fix this
	with RTLD_GLOBAL.

*/

#ifndef __simple_load_h_INCLUDE
#define __simple_load_h_INCLUDE

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <dlfcn.h>

/* This is the GNU name for the CTOR list */
#define CTOR_LIST       "__CTOR_LIST__"

#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT 0
#endif

/* Types defined appropriately for the dynamic linker */
typedef void* dl_handle_t;
typedef void* dl_symbol_t;

/* Do any initialization necessary.  Return 0 on success (or
   if no initialization needed. 
*/
static int 
__objc_dynamic_init(const char* exec_path)
{
  return 0;
}

/* Link in the module given by the name 'module'.  Return a handle which can
   be used to get information about the loded code.
*/
static dl_handle_t
__objc_dynamic_link(const char* module, int mode, const char* debug_file)
{
#ifdef RTLD_NOLOAD
  /*
   * If we've got RTLD_NOLOAD, then ask the dynamic linker first to check if
   * the library is already loaded.  If it is, then just return a handle to
   * it.  If not, then load it again.
   */
  void *handle = dlopen(module, RTLD_LAZY | RTLD_GLOBAL | RTLD_NOLOAD);
  if (NULL != handle)
    {
      return handle;
    }
#endif
  return (dl_handle_t)dlopen(module, RTLD_LAZY | RTLD_GLOBAL);
}

/* Return the address of a symbol given by the name 'symbol' from the module
 * associated with 'handle'
 * This function is not always used, so we mark it as unused to avoid warnings.
 */ 
static dl_symbol_t 
__objc_dynamic_find_symbol(dl_handle_t handle, const char* symbol)
    __attribute__((unused));
static dl_symbol_t 
__objc_dynamic_find_symbol(dl_handle_t handle, const char* symbol)
{
  return dlsym(handle, (char*)symbol);
}

/* remove the code from memory associated with the module 'handle' */
static int 
__objc_dynamic_unlink(dl_handle_t handle)
{
  return dlclose(handle);
}

/* Print an error message (prefaced by 'error_string') relevant to the
   last error encountered
*/
static void 
__objc_dynamic_error(FILE *error_stream, const char *error_string)
{
  fprintf(error_stream, "%s:%s\n", error_string, dlerror());
}

/* Debugging:  define these if they are available */
static int 
__objc_dynamic_undefined_symbol_count(void)
{
  return 0;
}

static char** 
__objc_dynamic_list_undefined_symbols(void)
{
  return NULL;
}

static inline const char *
__objc_dynamic_get_symbol_path(dl_handle_t handle, dl_symbol_t symbol)
{
#ifdef HAVE_DLADDR
  dl_symbol_t sym;
  Dl_info     info;

  if (handle == 0)
    handle = RTLD_DEFAULT;

  sym = dlsym(handle, symbol);

  if (!sym)
    return NULL;

  if (!dladdr(sym, &info))
    return NULL;

  return info.dli_fname;
#else
  return NULL;
#endif
}

#endif /* __simple_load_h_INCLUDE */
