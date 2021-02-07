/** Declaration of additional methods for base additions

   Copyright (C) 2008 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/

#ifndef	INCLUDED_NSTask_GNUstepBase_h
#define	INCLUDED_NSTask_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSTask.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

@interface	NSTask (GNUstepBase)
/** Returns the set of extensions known to indicate an executable file
 * type on systems which require that (currently mswindows).
 */
+ (NSSet*) executableExtensions;

/** Checks the specified file to see if it is executable or if by
 * appending one of the +executableExtensions it can be made executable.
 * The return value is the actual executable path or nil if the file
 * cannot be executed.
 */
+ (NSString*) executablePath: (NSString*)aFile;

/** Returns the launch path for a tool given the name of a tool.<br />
 * Locates the tool by looking in the standard directories and,
 * if not found there, looking in the PATH set in the environment.<br />
 * On ms-windows, this also tries appending common executable path
 * extensions to the tool name in order to find it.<br />
 * Returns the path found, or nil if the tool could not be located.
 */
+ (NSString*) launchPathForTool: (NSString*)name;
@end

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSTask_GNUstepBase_h */

