/* test

   Copyright (C) 2005 Free Software Foundation, Inc.

   This file is part of the GNU Objective C User Interface library.

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

#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

int main (int argc, char** argv, char** env)
{
  id pool = [NSAutoreleasePool new];
  NSArray* arguments;
  NSProcessInfo* processInfo;
  NSString *model;

#ifdef LIB_FOUNDATION_LIBRARY
  [NSProcessInfo initializeWithArguments:argv count:argc environment:env];
#endif

  processInfo = [NSProcessInfo processInfo];
  arguments = [processInfo arguments];
  [NSApplication sharedApplication];
  if ([arguments count] < 2)
    {
      model = @"test.gmodel";
      if (![NSBundle loadNibNamed: model owner: NSApp]) 
	{
	  printf ("Cannot load Interface Modeller file!\n");
	  exit (1);
	}
    }
  else
    {
      NSDictionary *table;
      table = [NSDictionary dictionaryWithObject: NSApp forKey: @"NSOwner"];
      model = [arguments objectAtIndex: 1];
      if (![NSBundle loadNibFile: model
	       externalNameTable: table
		        withZone: [NSApp zone]])
	{
	  printf ("Cannot load Interface Modeller file!\n");
	  exit (1);
	}

    }

  [[NSApplication sharedApplication] run];
  printf ("exiting...\n");

  [pool release];
  exit (0);
  return 0;
}

