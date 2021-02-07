/* Implementation of class NSStoryboard
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory Casamento
   Date: Mon Jan 20 15:57:37 EST 2020

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

#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSUUID.h>
#import <Foundation/NSException.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSStoryboard.h"
#import "AppKit/NSWindowController.h"
#import "AppKit/NSViewController.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSNibDeclarations.h"

#import "GNUstepGUI/GSModelLoaderFactory.h"
#import "GSStoryboardTransform.h"
#import "GSFastEnumeration.h"

static NSStoryboard *__mainStoryboard = nil;

// The storyboard needs to set this information on controllers...
@interface NSWindowController (__StoryboardPrivate__)
- (void) _setOwner: (id)owner;
- (void) _setTopLevelObjects: (NSArray *)array;
- (void) _setSegueMap: (NSMapTable *)map;
- (void) _setStoryboard: (NSStoryboard *)storyboard;
@end

@interface NSViewController (__StoryboardPrivate__)
- (void) _setTopLevelObjects: (NSArray *)array;
- (void) _setSegueMap: (NSMapTable *)map;
- (void) _setStoryboard: (NSStoryboard *)storyboard;
@end

@implementation NSWindowController (__StoryboardPrivate__)
- (void) _setOwner: (id)owner
{
  _owner = owner; // weak
}

- (void) _setTopLevelObjects: (NSArray *)array
{
  _top_level_objects = array;
}

- (void) _setSegueMap: (NSMapTable *)map
{
  ASSIGN(_segueMap, map);
}

- (void) _setStoryboard: (NSStoryboard *)storyboard
{
  _storyboard = storyboard;
}
@end

@implementation NSViewController (__StoryboardPrivate__)
- (void) _setTopLevelObjects: (NSArray *)array
{
  _topLevelObjects = array;
}

- (void) _setSegueMap: (NSMapTable *)map
{
  ASSIGN(_segueMap, map);
}

- (void) _setStoryboard: (NSStoryboard *)storyboard
{
  _storyboard = storyboard;
}
@end
// end private methods...

@implementation NSStoryboard

// Private instance methods...
- (id) initWithName: (NSStoryboardName)name
             bundle: (NSBundle *)bundle
{
  self = [super init];
  if (self != nil)
    {
      NSString *path = [bundle pathForResource: name
                                        ofType: @"storyboard"];
      NSData *data = [NSData dataWithContentsOfFile: path];
      _transform = [[GSStoryboardTransform alloc] initWithData: data];
    }
  return self;
}

// Class methods...
+ (void) _setMainStoryboard: (NSStoryboard *)storyboard  // private, only called from NSApplicationMain()
{
  if (__mainStoryboard == nil)
    {
      ASSIGN(__mainStoryboard, storyboard);
    }
}

+ (NSStoryboard *) mainStoryboard
{
  return __mainStoryboard;
}

+ (instancetype) storyboardWithName: (NSStoryboardName)name
                             bundle: (NSBundle *)bundle
{
  return AUTORELEASE([[NSStoryboard alloc] initWithName: name
                                                 bundle: bundle]);
}

// Instance methods...
- (void) dealloc
{
  RELEASE(_transform);
  [super dealloc];
}

- (void) _instantiateApplicationScene
{
  [self instantiateControllerWithIdentifier: @"application"];
}

- (id) instantiateInitialController
{
  return [self instantiateInitialControllerWithCreator: nil];
}

- (id) instantiateInitialControllerWithCreator: (NSStoryboardControllerCreator)block
{
  return [self instantiateControllerWithIdentifier: [_transform initialViewControllerId]
                                           creator: block];
}

- (id) instantiateControllerWithIdentifier: (NSStoryboardSceneIdentifier)identifier
{
  return [self instantiateControllerWithIdentifier: identifier
                                           creator: nil];
}

- (id) instantiateControllerWithIdentifier: (NSStoryboardSceneIdentifier)identifier
                                   creator: (NSStoryboardControllerCreator)block
{
  id result = nil;
  NSMutableArray *topLevelObjects = [NSMutableArray arrayWithCapacity: 5];
  NSDictionary *table = [NSDictionary dictionaryWithObjectsAndKeys: topLevelObjects,
                                      NSNibTopLevelObjects,
                                      NSApp,
                                      NSNibOwner,
                                      nil];
  GSModelLoader *loader = [GSModelLoaderFactory modelLoaderForFileType: @"xib"];
  BOOL  success = [loader loadModelData: [_transform dataForIdentifier: identifier]
                      externalNameTable: table
                               withZone: [self zone]];
    
  if (success)
    {
      NSMutableArray *seguesToPerform = [NSMutableArray array];
      NSMapTable *segueMap = [_transform segueMapForIdentifier: identifier];
      NSWindowController *wc = nil;
      NSViewController *vc = nil;
      NSWindow *w = nil;
            
      FOR_IN(id, o, topLevelObjects)
        if ([o isKindOfClass: [NSWindowController class]])
          {
            wc = (NSWindowController *)o;
            [wc _setSegueMap: segueMap];
            [wc _setTopLevelObjects: topLevelObjects];
            [wc _setStoryboard: self];
            [wc _setOwner: NSApp];
            result = o;
          }
        else if ([o isKindOfClass: [NSViewController class]])
          {
            vc = (NSViewController *)o;
            [vc _setSegueMap: segueMap];
            [vc _setTopLevelObjects: topLevelObjects];
            [vc _setStoryboard: self];
            result = o;
          }
        else if ([o isKindOfClass: [NSWindow class]])
          {
            w = (NSWindow *)o;
          }
        else if ([o isKindOfClass: [NSControllerPlaceholder class]])
          {
            NSControllerPlaceholder *ph = (NSControllerPlaceholder *)o;
            result = [ph instantiate];
          }             
      END_FOR_IN(topLevelObjects);

      // Process action proxies after so we know we have the windowController...
      FOR_IN(id, o, topLevelObjects)
        if ([o isKindOfClass: [NSStoryboardSeguePerformAction class]])
          {
            NSStoryboardSeguePerformAction *ssa = (NSStoryboardSeguePerformAction *)o;
            NSMapTable *mapTable = [_transform segueMapForIdentifier: identifier];
            NSStoryboardSegue *ss = [mapTable objectForKey: [ssa identifier]];
            
            [ssa setSender: result]; // resolve controller here...
            [ssa setStoryboardSegue: ss];
            [ssa setStoryboard: self];
            if ([[ssa kind] isEqualToString: @"relationship"]) // if it is a relationship, perform immediately
              {
                [seguesToPerform addObject: ssa];
              }
          }
      END_FOR_IN(topLevelObjects);

      // Depending on which kind of controller we have, do the correct thing....
      if (w != nil && wc != nil)
        {
          [wc setWindow: w];
        }

      // perform segues after all is initialized.
      FOR_IN(NSStoryboardSeguePerformAction*, ssa, seguesToPerform)
        [ssa doAction: result];  // this will, as far as I know, only happen with window controllers, to set content.
      END_FOR_IN(seguesToPerform);
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"Couldn't load controller scene identifier = %@", identifier];
    }

  // Execute the block if it's set...
  if (block != nil)
    {
      CALL_BLOCK(block, self);
    }
  return result;
}
@end
