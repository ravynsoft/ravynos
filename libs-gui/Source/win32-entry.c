/*
   DLL entry routine

   Copyright (C) 1997 Free Software Foundation, Inc.

   Original Author:  Scott Christley <scottc@net-community.com>
   Created: August 1997
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#include <windows.h>

/* Only if using Microsoft's tools and libraries */
#ifdef __MS_WIN32__
#include <stdio.h>
WINBOOL WINAPI _CRT_INIT( HINSTANCE hinstDLL, DWORD fdwReason,
			  LPVOID lpReserved );

// Global errno isn't defined in Microsoft's thread safe C library
void errno()
{}

int _MB_init_runtime()
{
    return 0;
}
#endif /* __MS_WIN32__ */

//
// DLL entry function for GNUstep GUI Library
// This function gets called everytime a process/thread attaches to DLL
//
WINBOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call,
		       LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
#ifdef __MS_WIN32__
	    /* Initialize the Microsoft C stdio DLL */
	    _CRT_INIT(hInst, ul_reason_for_call, lpReserved);

	    /* Initialize the GNUstep Base Library runtime structures */
	    gnustep_base_init_runtime();
#endif /* __MS_WIN32__ */
	}

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
	}

    if (ul_reason_for_call == DLL_THREAD_ATTACH)
	{
#ifdef __MS_WIN32__
	    /* Initialize C stdio DLL */
	    _CRT_INIT(hInst, ul_reason_for_call, lpReserved);
#endif /* __MS_WIN32__ */
	}

    if (ul_reason_for_call == DLL_THREAD_DETACH)
	{
	}

    return TRUE;
}
