/*
   Translator.m

   Translate a NIB file to a GNU model file.

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: November 1997
   
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

/* The original idea comes from the nib translator in objcX, an Objective-C
   class library for Motif, translator whose authors are Paul Kunz and
   Imran Qureshi.
 */

#include <AppKit/AppKit.h>
#include "GNUstepGUI/GMArchiver.h"
#include "GNUstepGUI/IMLoading.h"
#include "IBClasses.h"
#include "Translator.h"

NSMutableArray* objects;
NSMutableArray* connections;

@implementation Translator

- (void)translateNibFile:(NSString*)nibFile toModelFile:(NSString*)modelFile
{
  GMArchiver* archiver = [[GMArchiver new] autorelease];
  GMModel* model = [[GMModel new] autorelease];

  objects = [[NSMutableArray new] autorelease];
  connections = [[NSMutableArray new] autorelease];
  gmodelFile = [modelFile retain];

  [NSApplication sharedApplication];
  if (![NSBundle loadNibFile:nibFile
		externalNameTable:[NSDictionary dictionaryWithObjectsAndKeys:
					  NSApp, @"NSOwner", nil]
		withZone:[NSApp zone]]) {
    NSLog (@"Cannot load nib file %@!", nibFile);
    exit (1);
  }

  [model _setObjects:objects connections:connections];
  [archiver encodeRootObject:model withName:@"RootObject"];
  if (![archiver writeToFile:gmodelFile])
    NSLog (@"cannot write the model output file %@", gmodelFile);
}

@end
