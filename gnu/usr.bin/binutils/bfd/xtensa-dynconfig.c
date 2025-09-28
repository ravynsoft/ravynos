/* Xtensa configuration settings loader.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#define XTENSA_CONFIG_DEFINITION
#include "xtensa-config.h"
#include "xtensa-dynconfig.h"

#if defined (HAVE_DLFCN_H)
#include <dlfcn.h>
#elif defined (HAVE_WINDOWS_H)
#include <windows.h>
#endif

#if !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)

#define RTLD_LAZY 0      /* Dummy value.  */

static void *
dlopen (const char *file, int mode ATTRIBUTE_UNUSED)
{
  return LoadLibrary (file);
}

static void *
dlsym (void *handle, const char *name)
{
  return (void *) GetProcAddress ((HMODULE) handle, name);
}

static int ATTRIBUTE_UNUSED
dlclose (void *handle)
{
  FreeLibrary ((HMODULE) handle);
  return 0;
}

static const char *
dlerror (void)
{
  return _("Unable to load DLL.");
}

#endif /* !defined (HAVE_DLFCN_H) && defined (HAVE_WINDOWS_H)  */

#define CONFIG_ENV_NAME "XTENSA_GNU_CONFIG"

const void *xtensa_load_config (const char *name ATTRIBUTE_UNUSED,
				const void *no_plugin_def,
				const void *no_name_def ATTRIBUTE_UNUSED)
{
  static int init;
#if BFD_SUPPORTS_PLUGINS
  static void *handle;
  void *p;

  if (!init)
    {
      const char *path = getenv (CONFIG_ENV_NAME);

      init = 1;
      if (!path)
	return no_plugin_def;
      handle = dlopen (path, RTLD_LAZY);
      if (!handle)
	{
	  _bfd_error_handler (_("%s is defined but could not be loaded: %s"),
			      CONFIG_ENV_NAME, dlerror ());
	  abort ();
	}
    }
  else if (!handle)
    {
      return no_plugin_def;
    }

  p = dlsym (handle, name);
  if (!p)
    {
      if (no_name_def)
	return no_name_def;

      _bfd_error_handler (_("%s is loaded but symbol \"%s\" is not found: %s"),
			  CONFIG_ENV_NAME, name, dlerror ());
      abort ();
    }
  return p;
#else
  if (!init)
    {
      const char *path = getenv (CONFIG_ENV_NAME);

      init = 1;
      if (path)
	{
	  _bfd_error_handler (_("%s is defined but plugin support is disabled"),
			      CONFIG_ENV_NAME);
	  abort ();
	}
    }
  return no_plugin_def;
#endif
}

XTENSA_CONFIG_INSTANCE_LIST;

const struct xtensa_config_v1 *xtensa_get_config_v1 (void)
{
  static const struct xtensa_config_v1 *config;

  if (!config)
    config = (const struct xtensa_config_v1 *) xtensa_load_config ("xtensa_config_v1",
								   &xtensa_config_v1,
								   NULL);
  return config;
}

const struct xtensa_config_v2 *xtensa_get_config_v2 (void)
{
  static const struct xtensa_config_v2 *config;
  static const struct xtensa_config_v2 def;

  if (!config)
    config = (const struct xtensa_config_v2 *) xtensa_load_config ("xtensa_config_v2",
								   &xtensa_config_v2,
								   &def);
  return config;
}
