/* 
   GSNibCompatibility.h

   Copyright (C) 1997, 1999 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: 2002
   
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

#ifndef _GNUstep_H_GSNibCompatibility
#define _GNUstep_H_GSNibCompatibility

#import <Foundation/NSObject.h>

#import <AppKit/NSButton.h>
#import <AppKit/NSControl.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSMenu.h>
#import <AppKit/NSNibConnector.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSText.h>
#import <AppKit/NSTextView.h>
#import <AppKit/NSView.h>
#import <AppKit/NSWindow.h>

#import "GNUstepGUI/GSNibContainer.h"
#import "GNUstepGUI/GSInstantiator.h"

@class NSDictionary;
@class NSMapTable;
@class NSMutableArray;
@class NSMutableSet;
@class NSString;

// templates
@protocol OSXNibTemplate
- (void) setClassName: (NSString *)className;
- (NSString *)className;
- (void) setRealObject: (id)o;
- (id) realObject;
@end

@protocol GSNibLoading
- (id) nibInstantiate;
@end

typedef struct _GSWindowTemplateFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int isHiddenOnDeactivate:1;
  unsigned int isNotReleasedOnClose:1;
  unsigned int isDeferred:1;
  unsigned int isOneShot:1;
  unsigned int isVisible:1;
  unsigned int wantsToBeColor:1;
  unsigned int dynamicDepthLimit:1;
  unsigned int autoPositionMask:6;
  unsigned int savePosition:1;
  unsigned int style:2;
  unsigned int _unused2:3;
  unsigned int isNotShadowed:1;
  unsigned int autorecalculatesKeyViewLoop:1;
  unsigned int _unused:11; // currently not used, contains Cocoa specific info
#else
  unsigned int _unused:11; // currently not used, contains Cocoa specific info
  unsigned int autorecalculatesKeyViewLoop:1;
  unsigned int isNotShadowed:1;
  unsigned int _unused2:3;
  unsigned int style:2;
  unsigned int savePosition:1;
  unsigned int autoPositionMask:6;
  unsigned int dynamicDepthLimit:1;
  unsigned int wantsToBeColor:1;
  unsigned int isVisible:1;
  unsigned int isOneShot:1;
  unsigned int isDeferred:1;
  unsigned int isNotReleasedOnClose:1;
  unsigned int isHiddenOnDeactivate:1;
#endif
} GSWindowTemplateFlags;

// help connector class...
@interface NSIBHelpConnector : NSNibConnector
{
  id _marker;
  id _file;
}
- (void) setFile: (id)file;
- (id) file;
- (void) setMarker: (id)file;
- (id) marker;
@end
/**
 * Button image source class.
 */
@interface NSButtonImageSource : NSObject <NSCoding>
{
  NSString *imageName;
}
- (id) initWithImageNamed: (NSString *)name;
- (NSString *)imageName;
@end

/**
 * This class acts as a placeholder for the window.  It doesn't derive from
 * NSWindow for two reasons. First, it shouldn't instantiate a window immediately
 * when it's unarchived and second, it holds certain attributes (but doesn't set them
 * on the window, when the window is being edited in the application builder.
 */
@interface NSWindowTemplate : NSObject <OSXNibTemplate, NSCoding, GSNibLoading>
{
  NSBackingStoreType   _backingStoreType;
  NSSize               _maxSize;
  NSSize               _minSize;
  unsigned             _windowStyle;
  NSString            *_title;
  NSString            *_viewClass;
  NSString            *_windowClass;
  NSRect               _windowRect;
  NSRect               _screenRect;
  id                   _realObject;
  id                   _view;
  GSWindowTemplateFlags _flags;
  NSString            *_autosaveName;
  Class               _baseWindowClass;
  NSToolbar           *_toolbar;
}
- (id) initWithWindow: (NSWindow *)window
	    className: (NSString *)windowClass
           isDeferred: (BOOL) deferred
	    isOneShot: (BOOL) oneShot
	    isVisible: (BOOL) visible
       wantsToBeColor: (BOOL) wantsToBeColor
     autoPositionMask: (int) autoPositionMask;
- (void) setBackingStoreType: (NSBackingStoreType)type;
- (NSBackingStoreType) backingStoreType;
- (void) setDeferred: (BOOL)flag;
- (BOOL) isDeferred;
- (void) setMaxSize: (NSSize)maxSize;
- (NSSize) maxSize;
- (void) setMinSize: (NSSize)minSize;
- (NSSize) minSize;
- (void) setWindowStyle: (unsigned)sty;
- (unsigned) windowStyle;
- (void) setTitle: (NSString *) title;
- (NSString *)title;
- (void) setViewClass: (NSString *)viewClass;
- (NSString *)viewClass;
- (void) setWindowRect: (NSRect)rect;
- (NSRect)windowRect;
- (void) setScreenRect: (NSRect)rect;
- (NSRect) screenRect;
- (void) setView: (id)view;
- (id) view;
- (Class) baseWindowClass;
@end

@interface NSViewTemplate : NSView <OSXNibTemplate, NSCoding>
{
  NSString            *_className;
  id                   _realObject;
}
- (id) initWithObject: (id)o
            className: (NSString *)name;
@end

@interface NSTextTemplate : NSViewTemplate
{
}
@end

@interface NSTextViewTemplate : NSViewTemplate
{
}
@end

@interface NSMenuTemplate : NSObject <OSXNibTemplate, NSCoding>
{
  NSString            *_menuClass;
  NSString            *_title;
  id                   _realObject;
  id                   _parentMenu;
  NSPoint              _location;
  BOOL                 _isWindowsMenu;
  BOOL                 _isServicesMenu;
  BOOL                 _isFontMenu;
  NSInterfaceStyle     _interfaceStyle;
}
- (void) setClassName: (NSString *)name;
- (NSString *)className;
@end

@interface NSCustomObject : NSObject <NSCoding, GSNibLoading>
{
  NSString *_className;
  NSString *_extension;
  id _object;
}
- (void) setClassName: (NSString *)name;
- (NSString *)className;
- (void) setExtension: (NSString *)ext;
- (NSString *)extension;
- (void) setRealObject: (id)obj;
- (id) realObject;
@end

@interface NSCustomView : NSView <GSNibLoading>
{
  NSString *_className;
  NSString *_extension;
  NSView *_superview;
  NSView *_view;
}
- (void) setClassName: (NSString *)name;
- (NSString *)className;
- (void) setExtension: (NSString *)view;
- (NSString *)extension;
- (id)nibInstantiateWithCoder: (NSCoder *)coder;
@end

@interface NSCustomResource : NSObject <NSCoding>
{
  NSString *_className;
  NSString *_resourceName;
}
- (void) setClassName: (NSString *)className;
- (NSString *)className;
- (void) setResourceName: (NSString *)view;
- (NSString *)resourceName;
@end

@interface NSClassSwapper : NSObject <NSCoding>
{
  NSString *_className;
  NSString *_originalClassName;
  id _template;
}
- (id) initWithObject: (id)object 
        withClassName: (NSString *)className
    originalClassName: (NSString *)origClassName;
+ (void) setIsInInterfaceBuilder: (BOOL)flag;
+ (BOOL) isInInterfaceBuilder;
- (void) setTemplate: (id)temp;
- (id) template;
- (void) setClassName: (NSString *)className;
- (NSString *)className;
- (void) setOriginalClassName: (NSString *)className;
- (NSString *)originalClassName;
@end

@interface NSIBObjectData : NSObject <NSCoding, GSInstantiator, GSNibContainer>
{
  id              _root;
  NSMapTable     *_objects;
  NSMapTable     *_names;
  NSMapTable     *_oids;
  NSMapTable     *_classes;
  NSMapTable     *_instantiatedObjs;
  NSMutableArray *_visibleWindows;
  NSMutableArray *_connections;
  id              _firstResponder;
  id              _fontManager;
  NSString       *_framework;
  unsigned        _nextOid;
  NSMutableArray *_accessibilityConnectors;
  NSMapTable     *_accessibilityOids;
  NSMutableSet   *_topLevelObjects;
}
- (id) instantiateObject: (id)obj;
- (void) nibInstantiateWithOwner: (id)owner topLevelObjects: (NSMutableArray *)toplevel;
- (id) objectForName: (NSString *)name;
- (NSString *) nameForObject: (id)name;
- (NSMapTable *) objects;
- (NSMapTable *) names;
- (NSMapTable *) classes;
- (NSMapTable *) oids;
- (NSMutableArray *) visibleWindows;
- (void) setRoot: (id)root;
- (id) root;
- (void) setNextOid: (int)noid;
- (int) nextOid;
@end

// class needed for nib encoding/decoding by the progress bar...
@interface NSPSMatrix : NSObject
@end

@interface NSNibAXAttributeConnector : NSObject <NSCoding>
{
  NSString *_attributeType;
  NSString *_attributeValue;
  id _destination;
  id _source;
  NSString *_label;
}

// Attribute name/type.
- (NSString *) attributeType;
- (void) setAttributeType: (NSString *)type;
- (NSString *) attributeValue;
- (void) setAttributeValue: (NSString *)value;

// Source destination, connectors.
- (id) destination;
- (void) setDestination: (id)destination;
- (NSString *) label;
- (void) setLabel: (NSString *)label;
- (id) source;
- (void) setSource: (id)source;

// establish connection...
- (void) establishConnection;
@end

@interface NSNibAXRelationshipConnector : NSNibConnector
@end

@interface NSNibBindingConnector: NSNibConnector
{
  NSDictionary *_options;
  NSString *_binding;
  NSString *_keyPath;
  BOOL _hasEstablishedConnection;
}

- (NSString *) binding;
- (NSString *) keyPath;
- (NSDictionary *) options;
- (void) setBinding: (NSString *)bindings;
- (void) setKeyPath: (NSString *)keyPath;
- (void) setOptions: (NSDictionary *)options;
@end

@interface NSIBUserDefinedRuntimeAttributesConnector : NSObject <NSCoding>
{
  id _object;
  NSArray *_keyPaths;
  NSArray *_values;
}

- (void) instantiateWithObjectInstantiator: (id)instantiator;
- (void) establishConnection;
/*
- (void) replaceObject: (id)anObject withObject: (id)anotherObject;
- (void) setLabel: (id)label;
- (id) label;
- (void) setDestination: (id)destination;
- (id) destination;
- (void) setSource: (id)source;
- (id) source;
*/
- (void) setObject: (id)object;
- (id) object;
- (void) setValues: (id)values;
- (id) values;
- (void) setKeyPaths: (id)keyPaths;
- (id) keyPaths;
@end

#endif /* _GNUstep_H_GSNibCompatibility */
