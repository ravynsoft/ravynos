/* Control of exported symbols from libintl.
   Copyright (C) 2005-2022 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#if @HAVE_VISIBILITY@ && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __attribute__((__visibility__("default")))
#elif (defined _WIN32 && !defined __CYGWIN__) && defined WOE32DLL && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __declspec(dllexport)
#elif (defined _WIN32 && !defined __CYGWIN__) && defined WOE32DLL
#define LIBINTL_DLL_EXPORTED __declspec(dllimport)
#else
#define LIBINTL_DLL_EXPORTED
#endif
