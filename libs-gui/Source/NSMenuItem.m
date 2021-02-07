/** <title>NSMenuItem</title>

   <abstract>The menu cell class.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: May 1997
   Author:  David Lazaro Saz <khelekir@encomix.es>
   Date: Sep 1999
   
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

#import "config.h"
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSMenuItem.h"
#import "AppKit/NSMenu.h"
#import "GSBindingHelpers.h"

static BOOL usesUserKeyEquivalents = NO;
static Class imageClass;

@interface GSMenuSeparator : NSMenuItem

@end

@implementation GSMenuSeparator

- (Class) classForCoder
{
  return [NSMenuItem class];
}

- (id) init
{
  self = [super initWithTitle: @"-----------"
		action: NULL
		keyEquivalent: @""];
  if (self == nil)
    return nil;

  _enabled = NO;
  _changesState = NO;
  return self;
}

- (BOOL) isSeparatorItem
{
  return YES;
}

// FIXME: We need a lot of methods to switch off changes for a separator
@end


@implementation NSMenuItem

+ (void) initialize
{
  if (self == [NSMenuItem class])
    {
      [self setVersion: 4];
      imageClass = [NSImage class];

      [self exposeBinding: NSEnabledBinding];
    }
}

+ (void) setUsesUserKeyEquivalents: (BOOL)flag
{
  usesUserKeyEquivalents = flag;
}

+ (BOOL) usesUserKeyEquivalents
{
  return usesUserKeyEquivalents;
}

+ (id <NSMenuItem>) separatorItem
{
  return AUTORELEASE([GSMenuSeparator new]);
}

- (id) init
{
  return [self initWithTitle: @""
	       action: NULL
	       keyEquivalent: @""];
}

- (void) dealloc
{
  // Remove all key value bindings for this view.
  [GSKeyValueBinding unbindAllForObject: self];

  TEST_RELEASE(_title);
  TEST_RELEASE(_keyEquivalent);
  TEST_RELEASE(_image);
  TEST_RELEASE(_onStateImage);
  TEST_RELEASE(_offStateImage);
  TEST_RELEASE(_mixedStateImage);
  TEST_RELEASE(_submenu);
  TEST_RELEASE(_representedObject);
  TEST_RELEASE(_toolTip);
  [super dealloc];
}

- (id) initWithTitle: (NSString*)aString
	      action: (SEL)aSelector
       keyEquivalent: (NSString*)charCode
{
  self = [super init];
  if (self == nil)
    return nil;

  //_menu = nil;
  [self setKeyEquivalent: charCode];
  _keyEquivalentModifierMask = NSCommandKeyMask;
  [self setTitle: aString]; // do this AFTER setKeyEquivalent: in case NSUserKeyEquivalents is defined
  _mnemonicLocation = 255; // No mnemonic
  _state = NSOffState;
  _enabled = YES;
  //_image = nil;
  // Set the images according to the spec. On: check mark; off: dash.
  [self setOnStateImage: [imageClass imageNamed: @"NSMenuCheckmark"]];
  [self setMixedStateImage: [imageClass imageNamed: @"NSMenuMixedState"]];
  //_offStateImage = nil;
  //_target = nil;
  _action = aSelector;
  //_changesState = NO;
  return self;
}

- (void) _updateKeyEquivalent
{
  // Update keyEquivalent based on any entries in NSUserKeyEquivalents in the defaults database
  // TODO: also check in other defaults domains, to allow NSUserKeyEquivalents dictionaries
  // in different domains to provide overrides of different menu items.
  NSString *userKeyEquivalent = [(NSDictionary*)[[NSUserDefaults standardUserDefaults]
				     objectForKey: @"NSUserKeyEquivalents"]
				    objectForKey: _title];
  if (userKeyEquivalent)
    {
      // check for leading symbols representing modifier flags: @, ~, $, ^
      NSUInteger modifierMask = 0;
      while ([userKeyEquivalent length] > 1)
        {
          if ([userKeyEquivalent hasPrefix:@"@"])
            {
              modifierMask |= NSCommandKeyMask;
              userKeyEquivalent = [userKeyEquivalent substringFromIndex:1];
            }
          else if ([userKeyEquivalent hasPrefix:@"~"])
            {
              modifierMask |= NSAlternateKeyMask;
              userKeyEquivalent = [userKeyEquivalent substringFromIndex:1];
            }
          else if ([userKeyEquivalent hasPrefix:@"$"])
            {
              modifierMask |= NSShiftKeyMask;
              userKeyEquivalent = [userKeyEquivalent substringFromIndex:1];
            }
          else if ([userKeyEquivalent hasPrefix:@"^"])
            {
              modifierMask |= NSControlKeyMask;
              userKeyEquivalent = [userKeyEquivalent substringFromIndex:1];
            }
          else
            {
              break;
            }
        }
      [self setKeyEquivalent:userKeyEquivalent];
      [self setKeyEquivalentModifierMask:modifierMask];
    }
}

- (void) setMenu: (NSMenu*)menu
{
  /* The menu is retaining us.  Do not retain it.  */
  _menu = menu;
  if (_submenu != nil)
    {
      [_submenu setSupermenu: menu];
      [self setTarget: _menu];
    }
}

- (NSMenu*) menu
{
  return _menu;
}

- (BOOL) hasSubmenu
{
  return (_submenu == nil) ? NO : YES;
}

- (void) setSubmenu: (NSMenu*)submenu
{
  if (submenu == _submenu)
    return; // no change
	
  if ([submenu supermenu] != nil)
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"submenu (%@) already has supermenu (%@)",
		   [submenu title], [[submenu supermenu] title]];
    }
  ASSIGN(_submenu, submenu);
  if (submenu != nil)
    {
      [submenu setSupermenu: _menu];
      [submenu setTitle: _title];
    }
  [self setTarget: _menu];
  [self setAction: @selector(submenuAction:)];
  [_menu itemChanged: self];
}

- (NSMenu*) submenu
{
  return _submenu;
}

- (void) setTitle: (NSString*)aString
{
  if (nil == aString)
    aString = @"";
	
  if ([_title isEqualToString:aString])
    return; // no change
	
  ASSIGNCOPY(_title,  aString);
  [self _updateKeyEquivalent];
  [_menu itemChanged: self];
}

- (NSString*) title
{
  return _title;
}

- (BOOL) isSeparatorItem
{
  return NO;
}

- (void) setKeyEquivalent: (NSString*)aKeyEquivalent
{
  if (nil == aKeyEquivalent)
    {
      /* We warn about nil for compatibiliy with MacOS X, which refuses
         nil.  */
      NSDebugMLLog(@"MacOSXCompatibility", 
                   @"Attempt to use nil as key equivalent");
      aKeyEquivalent = @"";
    }
  if ([_keyEquivalent isEqualToString:aKeyEquivalent])
    return; // no change
	
  ASSIGNCOPY(_keyEquivalent,  aKeyEquivalent);
  [_menu itemChanged: self];
}

- (NSString*) keyEquivalent
{
  if (usesUserKeyEquivalents)
    return [self userKeyEquivalent];
  else
    return _keyEquivalent;
}

- (void) setKeyEquivalentModifierMask: (NSUInteger)mask
{
  if (_keyEquivalentModifierMask == mask)
    return; // no change
  _keyEquivalentModifierMask = mask;
  [_menu itemChanged: self];
}

- (NSUInteger) keyEquivalentModifierMask
{
  return _keyEquivalentModifierMask;
}

- (NSString*) userKeyEquivalent
{
  NSString *userKeyEquivalent = [(NSDictionary*)[[[NSUserDefaults standardUserDefaults]
				      persistentDomainForName: NSGlobalDomain]
				     objectForKey: @"NSCommandKeys"]
				    objectForKey: _title];

  if (nil == userKeyEquivalent)
    userKeyEquivalent = @"";

  return userKeyEquivalent;
}

- (NSUInteger) userKeyEquivalentModifierMask
{
  // FIXME
  return NSCommandKeyMask;
}

- (void) setMnemonicLocation: (NSUInteger)location
{
  if (_mnemonicLocation == location)
    return; // no change
	
  _mnemonicLocation = location;
  [_menu itemChanged: self];
}

- (NSUInteger) mnemonicLocation
{
  if (_mnemonicLocation != 255)
    return _mnemonicLocation;
  else
    return NSNotFound;
}

- (NSString*) mnemonic
{
  if (_mnemonicLocation != 255)
    return [_title substringWithRange: NSMakeRange(_mnemonicLocation, 1)];
  else
    return @"";
}

- (void) setTitleWithMnemonic: (NSString*)stringWithAmpersand
{
  NSUInteger location = [stringWithAmpersand rangeOfString: @"&"].location;

  [self setTitle: [stringWithAmpersand stringByReplacingString: @"&"
				       withString: @""]];
  [self setMnemonicLocation: location];
}

- (void) setImage: (NSImage *)image
{
  NSAssert(image == nil || [image isKindOfClass: imageClass],
    NSInvalidArgumentException);
	
  if (_image == image)
    return; // no change
	
  ASSIGN(_image, image);
  [_menu itemChanged: self];
}

- (NSImage*) image
{
  return _image;
}

- (void) setState: (NSInteger)state
{
  if (_state == state)
    return;

  _state = state;
  _changesState = YES;
  [_menu itemChanged: self];
}

- (NSInteger) state
{
  return _state;
}

- (void) setOnStateImage: (NSImage*)image
{
  NSAssert(image == nil || [image isKindOfClass: imageClass],
    NSInvalidArgumentException);
	
  if (_onStateImage == image)
    return; // no change
	
  ASSIGN(_onStateImage, image);
  [_menu itemChanged: self];
}

- (NSImage*) onStateImage
{
  return _onStateImage;
}

- (void) setOffStateImage: (NSImage*)image
{
  NSAssert(image == nil || [image isKindOfClass: imageClass],
    NSInvalidArgumentException);
	
  if (_offStateImage == image)
    return; // no change

  ASSIGN(_offStateImage, image);
  [_menu itemChanged: self];
}

- (NSImage*) offStateImage
{
  return _offStateImage;
}

- (void) setMixedStateImage: (NSImage*)image
{
  NSAssert(image == nil || [image isKindOfClass: imageClass],
    NSInvalidArgumentException);
	
  if (_mixedStateImage == image)
    return; // no change
	
  ASSIGN(_mixedStateImage, image);
  [_menu itemChanged: self];
}

- (NSImage*) mixedStateImage
{
  return _mixedStateImage;
}

- (void) setEnabled: (BOOL)flag
{
  if (flag == _enabled)
    return;

  _enabled = flag;
  [_menu itemChanged: self];
}

- (BOOL) isEnabled
{
  return _enabled;
}

- (void) setTarget: (id)anObject
{
  if (_target == anObject)
    return;

  _target = anObject;
  [_menu itemChanged: self];
}

- (id) target
{
  return _target;
}

- (void) setAction: (SEL)aSelector
{
  if (_action == aSelector)
    return;

  _action = aSelector;
  [_menu itemChanged: self];
}

- (SEL) action
{
  return _action;
}

- (void) setTag: (NSInteger)anInt
{
  _tag = anInt;
}

- (NSInteger) tag
{
  return _tag;
}

- (void) setRepresentedObject: (id)anObject
{
  ASSIGN(_representedObject, anObject);
}

- (id) representedObject
{
  return _representedObject;
}

- (NSAttributedString *)attributedTitle
{
  // FIXME
  return nil;
}

-(void) setAttributedTitle: (NSAttributedString *)title
{
  // FIXME
  [self setTitle: [title string]];
}

- (NSInteger)indentationLevel
{
  return _indentation;
}

- (void)setIndentationLevel: (NSInteger)level
{
  _indentation = level;
}

- (BOOL)isAlternate
{
  return _isAlternate;
}

- (void) setAlternate: (BOOL)isAlternate
{
  _isAlternate = isAlternate;
}

- (void) setToolTip: (NSString *)toolTip
{
  ASSIGN(_toolTip, toolTip);
}

- (NSString *) toolTip
{
  return _toolTip;
}

/*
 * NSCopying protocol
 */
- (id) copyWithZone: (NSZone*)zone
{
  NSMenuItem *copy = (NSMenuItem*)NSCopyObject (self, 0, zone);

  // We reset the menu to nil to allow the reuse of the copy
  copy->_menu = nil;
  copy->_title = [_title copyWithZone: zone];
  copy->_keyEquivalent = [_keyEquivalent copyWithZone: zone];
  copy->_image = [_image copyWithZone: zone];
  copy->_onStateImage = [_onStateImage copyWithZone: zone];
  copy->_offStateImage = [_offStateImage copyWithZone: zone];
  copy->_mixedStateImage = [_mixedStateImage copyWithZone: zone];
  copy->_representedObject = RETAIN(_representedObject);
  copy->_submenu = [_submenu copy];
  copy->_toolTip = RETAIN(_toolTip);
  copy->_target = _target;

  return copy;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      if ([self isSeparatorItem])
        {
          [aCoder encodeBool: YES forKey: @"NSIsSeparator"];
        }
      [aCoder encodeObject: _title forKey: @"NSTitle"];
      [aCoder encodeObject: NSStringFromSelector(_action) forKey: @"NSAction"];
      [aCoder encodeObject: _keyEquivalent forKey: @"NSKeyEquiv"];
      [aCoder encodeObject: _onStateImage forKey: @"NSOnImage"];
      [aCoder encodeObject: _offStateImage forKey: @"NSOffImage"]; // ???????
      [aCoder encodeObject: _mixedStateImage forKey: @"NSMixedImage"]; 
      [aCoder encodeObject: _target forKey: @"NSTarget"];
      [aCoder encodeObject: _menu forKey: @"NSMenu"];

      // If the menu is owned by a popup, then don't encode the children.
      // This prevents an assertion error in IB as these keys should not 
      // be present in a menu item when it's encoded as part of a popup. 
      if([_menu _ownedByPopUp] == NO)
	{
	  [aCoder encodeObject: _submenu forKey: @"NSSubmenu"];
	}

      [aCoder encodeInt: _keyEquivalentModifierMask forKey: @"NSKeyEquivModMask"];
      [aCoder encodeInt: _mnemonicLocation forKey: @"NSMnemonicLoc"];
      [aCoder encodeInt: _state forKey: @"NSState"];
      [aCoder encodeBool: ![self isEnabled] forKey: @"NSIsDisabled"];
      if (_tag)
        {
          [aCoder encodeInt: _tag forKey: @"NSTag"];
        }
    }
  else
    {
      [aCoder encodeObject: _title];
      [aCoder encodeObject: _keyEquivalent];
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_keyEquivalentModifierMask];
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_mnemonicLocation];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_state];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_enabled];
      [aCoder encodeObject: _image];
      [aCoder encodeObject: _onStateImage];
      [aCoder encodeObject: _offStateImage];
      [aCoder encodeObject: _mixedStateImage];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_changesState];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_action];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_tag];
      [aCoder encodeConditionalObject: _representedObject];
      [aCoder encodeObject: _submenu];
      [aCoder encodeConditionalObject: _target];
      
      // version 3
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isAlternate];
      [aCoder encodeValueOfObjCType: @encode(char) at: &_indentation];
      [aCoder encodeObject: _toolTip];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSString *title;
      NSString *action;
      NSString *key;
      BOOL isSeparator = NO;
      int keyMask;

      if ([aDecoder containsValueForKey: @"NSIsSeparator"])
        {
          isSeparator = [aDecoder decodeBoolForKey: @"NSIsSeparator"];
        }

      if (isSeparator && ![self isSeparatorItem])
        {
          RELEASE(self);

	  //
	  // An object returned from initWithCoder: 
	  // should not be autoreleased.  Do a retain
	  // to prevent it from being released automatically.
	  //
          self = RETAIN([NSMenuItem separatorItem]);
        }

      //
      // Not retained, because we're calling the designated init with
      // the values.
      //
      title = [aDecoder decodeObjectForKey: @"NSTitle"];
      action = [aDecoder decodeObjectForKey: @"NSAction"];
      key = [aDecoder decodeObjectForKey: @"NSKeyEquiv"];

      self = [self initWithTitle: title
                   action: NSSelectorFromString(action)
                   keyEquivalent: key];
      //[aDecoder decodeObjectForKey: @"NSMenu"];

      if ([aDecoder containsValueForKey: @"NSTarget"])
        {
         id target = [aDecoder decodeObjectForKey: @"NSTarget"];
         [self setTarget: target];
        }
      if ([aDecoder containsValueForKey: @"NSMixedImage"])
        {
          NSImage *mixedImage = [aDecoder decodeObjectForKey: @"NSMixedImage"];
          [self setMixedStateImage: mixedImage];
        }
      if ([aDecoder containsValueForKey: @"NSOnImage"])
        {
          NSImage *onImage = [aDecoder decodeObjectForKey: @"NSOnImage"];
          [self setOnStateImage: onImage];
        }
      if ([aDecoder containsValueForKey: @"NSSubmenu"])
        {
          NSMenu *submenu = [aDecoder decodeObjectForKey: @"NSSubmenu"];
          [self setSubmenu: submenu];
        }

      // Set the key mask regardless of whether it is present;
      // i.e. set it to 0 if it is not present in the nib.
      keyMask = [aDecoder decodeIntForKey: @"NSKeyEquivModMask"];
      [self setKeyEquivalentModifierMask: keyMask];

      if ([aDecoder containsValueForKey: @"NSMnemonicLoc"])
        {
          int loc = [aDecoder decodeIntForKey: @"NSMnemonicLoc"];
          [self setMnemonicLocation: loc];
        }
      if ([aDecoder containsValueForKey: @"NSState"])
        {
          _state = [aDecoder decodeIntForKey: @"NSState"];
        }
      if ([aDecoder containsValueForKey: @"NSIsDisabled"])
        {
          BOOL flag = [aDecoder decodeBoolForKey: @"NSIsDisabled"];
          [self setEnabled: !flag];
        }
      if ([aDecoder containsValueForKey: @"NSTag"])
        {
          int tag = [aDecoder decodeIntForKey: @"NSTag"];
          [self setTag: tag];
        }
    }
  else
    {
      int version = [aDecoder versionForClassName: 
				  @"NSMenuItem"];
    
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_title];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_keyEquivalent];
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger) at: &_keyEquivalentModifierMask];
      if (version <= 3)
        {
          _keyEquivalentModifierMask = _keyEquivalentModifierMask << 16;
        }
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger) at: &_mnemonicLocation];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_state];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_enabled];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_image];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_onStateImage];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_offStateImage];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_mixedStateImage];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_changesState];
      if (version == 1)
        {
          _target = [aDecoder decodeObject];
        }
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_action];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_tag];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_representedObject];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_submenu];
      if (version >= 2)
        {
          _target = [aDecoder decodeObject];
        }
      if (version >= 3)
        {
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isAlternate];
          [aDecoder decodeValueOfObjCType: @encode(char) at: &_indentation];
          [aDecoder decodeValueOfObjCType: @encode(id) at: &_toolTip];
        }
    }

  [self _updateKeyEquivalent];
  return self;
}

- (void) bind: (NSString *)binding
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding hasPrefix: NSEnabledBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueAndBinding alloc] initWithBinding: NSEnabledBinding 
                                                 withName: binding 
                                                 toObject: anObject
                                              withKeyPath: keyPath
                                                  options: options
                                               fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding
             toObject: anObject
             withKeyPath: keyPath
             options: options];
    }
}

@end

@implementation NSMenuItem (GNUstepExtra)

/*
 * These methods support the special arranging in columns of menu
 * items in GNUstep.  There's no need to use them outside but if
 * they are used the display is more pleasant.
 */
- (void) setChangesState: (BOOL)flag
{
  _changesState = flag;
}

- (BOOL) changesState
{
  return _changesState;
}

@end
