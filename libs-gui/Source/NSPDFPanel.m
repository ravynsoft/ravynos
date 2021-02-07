/* Implementation of class NSPDFPanel
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat Nov 16 21:21:00 EST 2019

   This file is part of the GNUstep Library.
   
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

#import <AppKit/NSPDFPanel.h>
#import <AppKit/NSPDFInfo.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSViewController.h>
#import <Foundation/NSString.h>

@implementation NSPDFPanel

+ (NSPDFPanel *) panel
{
  return [[self class] alloc] ;
}

- init
{
  self = [super init];
  if (self != nil)
    {
      _options = NSPDFPanelShowsPaperSize;
      _defaultFileName = @"";
    }
  return self;
}

- (NSViewController *) accessoryController
{
  return _accessoryController;
}

- (void) setAccessoryController: (NSViewController *)accessoryController
{
  ASSIGN(_accessoryController, accessoryController);
}

- (NSPDFPanelOptions) options
{
  return _options;
}

- (void) setOptions: (NSPDFPanelOptions)opts
{
  _options = opts;
}

- (NSString *) defaultFileName
{
  return _defaultFileName;
}

- (void) setDefaultFileName: (NSString *)fileName
{
  ASSIGNCOPY(_defaultFileName, fileName);
}

- (void) beginSheetWithPDFInfo: (NSPDFInfo *)pdfInfo
                modalForWindow: (NSWindow *)window
             completionHandler: (GSPDFPanelCompletionHandler)handler
{
}

@end

