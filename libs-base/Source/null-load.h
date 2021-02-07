/*
    null-load - NULL dynamic loader. Doesn't do anything.

    Copyright (C) 1995, Free Software Foundation, Inc.

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

*/

#ifndef __null_load_h_INCLUDE
#define __null_load_h_INCLUDE

#define CTOR_LIST ""

/* Types defined appropriately for the dynamic linker */
typedef void* dl_handle_t;
typedef void* dl_symbol_t;

/* Do any initialization necessary.  Return 0 on success (or
   if no initialization needed. 
*/
static int 
__objc_dynamic_init(const char* exec_path)
{
  return -1;
}

/* Link in the module given by the name 'module'.  Return a handle which can
   be used to get information about the loded code.
*/
static dl_handle_t
__objc_dynamic_link(const char* module, int mode, const char* debug_file)
{
  return 0;
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
  return 0;
}

/* remove the code from memory associated with the module 'handle' */
static int 
__objc_dynamic_unlink(dl_handle_t handle)
{
  return 0;
}

/* Print an error message (prefaced by 'error_string') relevant to the
   last error encountered
*/
static void 
__objc_dynamic_error(FILE *error_stream, const char *error_string)
{
  fprintf(error_stream, "%s\n", error_string);
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

static char *
__objc_dynamic_get_symbol_path(dl_handle_t handle, dl_symbol_t symbol)
{
  return NULL;
}

#endif /* __null_load_h_INCLUDE */
