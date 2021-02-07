/*
   GMAppKit.m

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

#import <Foundation/NSDebug.h>
#import "GNUstepGUI/GMAppKit.h"

#ifndef AUTORELEASE
#define AUTORELEASE(object)	[object autorelease]
#define RELEASE(object)		[object release]
#define RETAIN(object)		[object retain]
#endif

void __dummy_GMAppKit_functionForLinking() {}

@implementation NSApplication (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
#ifndef GNU_GUI_LIBRARY
  NSArray* windows1 = [self windows];
  NSMutableArray* windows2 = [NSMutableArray array];
  int i, count = [windows1 count];

  for (i = 0; i < count; i++) {
    NSWindow* window = [windows1 objectAtIndex:i];

    if (([window isKindOfClass:[NSMenu class]] == NO)
	&& ([NSStringFromClass ([window class]) 
	    isEqualToString: @"NSMenuPanel"] == NO))
      [windows2 addObject:window];
  }
  [archiver encodeObject:windows2 withName:@"windows"];

#else
  [archiver encodeObject:[self windows] withName:@"windows"];
#endif
  [archiver encodeObject:[self keyWindow] withName:@"keyWindow"];
  [archiver encodeObject:[self mainWindow] withName:@"mainWindow"];
  [archiver encodeObject:[self mainMenu] withName:@"mainMenu"];
  [archiver encodeObject:[self delegate] withName:@"delegate"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSArray* windows;
  NSEnumerator *enumerator;
  NSWindow *win;
  NSWindow* keyWindow;
  NSWindow* mainWindow;
  NSMenu* mainMenu;
  id anObject;

#if GNU_GUI_LIBRARY
  mainMenu = [unarchiver decodeObjectWithName:@"mainMenu"];
  if (mainMenu)
    [self setMainMenu:mainMenu];
#endif

  windows = [unarchiver decodeObjectWithName:@"windows"];
  enumerator = [windows objectEnumerator];
  while ((win = [enumerator nextObject]) != nil)
    {
      /* If we did not retain the windows here, they would all get 
	 released at the end of the event loop. */
      RETAIN (win);
    }

  keyWindow = [unarchiver decodeObjectWithName:@"keyWindow"];
  mainWindow = [unarchiver decodeObjectWithName:@"mainWindow"];

  anObject = [unarchiver decodeObjectWithName:@"delegate"];
  if (anObject)
    [self setDelegate:anObject];

#ifndef GNU_GUI_LIBRARY
  mainMenu = [unarchiver decodeObjectWithName:@"mainMenu"];
  if (mainMenu)
    [self setMainMenu:mainMenu];
#endif

  [keyWindow makeKeyWindow];
  [mainWindow makeMainWindow];

  return self;
}

- (void)awakeFromModel
{
  NSMenu* mainMenu = [self mainMenu];

  [mainMenu update];
#if XDPS_BACKEND_LIBRARY || XRAW_BACKEND_LIBRARY || XGPS_BACKEND_LIBRARY
  [mainMenu display];
#endif

}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return [NSApplication sharedApplication];
}

@end /* NSApplication (GMArchiverMethods) */


@implementation NSBox (GMArchiverMethods)

/* NSBox is very special because it always has a single subview, which
   is the contentview, and it overrides addSubview: to add subviews to
   the contentview.  Make sure we can manage this case properly and
   portably.  */
- (NSArray *)subviewsForModel
{
    return [NSArray array];
}

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeSize:[self contentViewMargins] withName:@"contentViewMargins"];
  [archiver encodeInt:[self borderType] withName:@"borderType"];
  [archiver encodeInt:[self titlePosition] withName:@"titlePosition"];
  [archiver encodeString:[self title] withName:@"title"];
  [archiver encodeObject:[self titleFont] withName:@"titleFont"];
  [archiver encodeObject:[self contentView] withName:@"contentView"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setContentViewMargins:[unarchiver decodeSizeWithName:@"contentViewMargins"]];
  [self setBorderType:[unarchiver decodeIntWithName:@"borderType"]];
  [self setTitlePosition:[unarchiver decodeIntWithName:@"titlePosition"]];
  [self setTitle:[unarchiver decodeStringWithName:@"title"]];
  [self setTitleFont:[unarchiver decodeObjectWithName:@"titleFont"]];
  [self setContentView: [unarchiver decodeObjectWithName:@"contentView"]];

  return self;
}

@end /* NSBox (GMArchiverMethods) */


#if 0
@implementation NSButton (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  float delay, interval;
  id theCell = [self cell];

  [archiver encodeInt:[self state] withName:@"state"];
  [self getPeriodicDelay:&delay interval:&interval];
  [archiver encodeFloat:delay withName:@"delay"];
  [archiver encodeFloat:interval withName:@"interval"];
  [archiver encodeString:[self title] withName:@"title"];
  [archiver encodeString:[self alternateTitle] withName:@"alternateTitle"];
  [archiver encodeObject:[self image] withName:@"image"];
  [archiver encodeObject:[self alternateImage] withName:@"alternateImage"];
  [archiver encodeInt:[self imagePosition] withName:@"imagePosition"];
  [archiver encodeBOOL:[self isBordered] withName:@"isBordered"];
  [archiver encodeBOOL:[self isTransparent] withName:@"isTransparent"];
  [archiver encodeString:[self keyEquivalent] withName:@"keyEquivalent"];
  [archiver encodeInt:[theCell highlightsBy] withName:@"highlightsBy"];
  [archiver encodeInt:[theCell showsStateBy] withName:@"showsStateBy"];

  [super encodeWithModelArchiver:archiver];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  float delay, interval;
  id theCell;

  self = [super initWithModelUnarchiver:unarchiver];

  [self setState:[unarchiver decodeIntWithName:@"state"]];

  delay = [unarchiver decodeFloatWithName:@"delay"];
  interval = [unarchiver decodeFloatWithName:@"interval"];
  [self setPeriodicDelay:delay interval:interval];

  [self setTitle:[unarchiver decodeStringWithName:@"title"]];
  [self setAlternateTitle:[unarchiver decodeStringWithName:@"alternateTitle"]];
  [self setImage:[unarchiver decodeObjectWithName:@"image"]];
  [self setAlternateImage:[unarchiver decodeObjectWithName:@"alternateImage"]];
  [self setImagePosition:[unarchiver decodeIntWithName:@"imagePosition"]];
  [self setBordered:[unarchiver decodeBOOLWithName:@"isBordered"]];
  [self setTransparent:[unarchiver decodeBOOLWithName:@"isTransparent"]];
  [self setKeyEquivalent:[unarchiver decodeStringWithName:@"keyEquivalent"]];

  theCell = [self cell];

  [theCell setHighlightsBy:[unarchiver decodeIntWithName:@"highlightsBy"]];
  [theCell setShowsStateBy:[unarchiver decodeIntWithName:@"showsStateBy"]];

  return self;
}

@end /* NSButton (GMArchiverMethods) */
#endif


@implementation NSCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeInt:[self type] withName:@"type"];
  [archiver encodeObject:[self font] withName:@"font"];
  [archiver encodeString:[self stringValue] withName:@"stringValue"];
  [archiver encodeInt:[self entryType] withName:@"entryType"];
  [archiver encodeInt:[self alignment] withName:@"alignment"];
  [archiver encodeBOOL:[self wraps] withName:@"wraps"];
  [archiver encodeObject:[self image] withName:@"image"];
  [archiver encodeInt:[self state] withName:@"state"];
  [archiver encodeBOOL:[self isEnabled] withName:@"isEnabled"];
  [archiver encodeBOOL:[self isBordered] withName:@"isBordered"];
  [archiver encodeBOOL:[self isBezeled] withName:@"isBezeled"];
  [archiver encodeBOOL:[self isEditable] withName:@"isEditable"];
  [archiver encodeBOOL:[self isSelectable] withName:@"isSelectable"];
  [archiver encodeBOOL:[self isScrollable] withName:@"isScrollable"];
  [archiver encodeBOOL:[self isContinuous] withName:@"isContinuous"];
  [archiver encodeInt:[self sendActionOn: 0] withName:@"sendActionMask"];
  {
    /* NB: this is not decoded so maybe we could just do without
       encoding it. :-) */
    int actionMask = [self sendActionOn: 0];
    [archiver encodeInt:actionMask withName:@"sendActionMask"];
    [self sendActionOn: actionMask];
  }
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  int cellType = [unarchiver decodeIntWithName:@"type"];
  NSFont* font = [unarchiver decodeObjectWithName:@"font"];
  
  // this is a tricky object to decode, because a number of its methods
  // have side-effects; [-setEntryType:] converts the cell to a text-type
  // cell and sets its font to the system font, so it comes first
  [self setEntryType:[unarchiver decodeIntWithName:@"entryType"]];
  
  // now set the font
  [self setFont:font];
  
  // both [-setImage:] and [-setStringValue:] convert the cell to an
  // image or text cell (respectively), so they must be called in the
  // correct order for the type of cell desired
  switch (cellType)
    {
    case NSTextCellType:
      [self setImage:[unarchiver decodeObjectWithName:@"image"]];
      [self setStringValue: 
	      [unarchiver decodeStringWithName:@"stringValue"]];
      break;
    case NSImageCellType:
      [self setStringValue:
              [unarchiver decodeStringWithName:@"stringValue"]];
      [self setImage:[unarchiver decodeObjectWithName:@"image"]];
      break;
    case NSNullCellType:
      [self setType: NSNullCellType];
      break;
    }
  [self setAlignment:[unarchiver decodeIntWithName:@"alignment"]];
  [self setWraps:[unarchiver decodeBOOLWithName:@"wraps"]];
  [self setState:[unarchiver decodeIntWithName:@"state"]];
  [self setEnabled:[unarchiver decodeBOOLWithName:@"isEnabled"]];
  [self setBordered:[unarchiver decodeBOOLWithName:@"isBordered"]];
  [self setBezeled:[unarchiver decodeBOOLWithName:@"isBezeled"]];
  [self setEditable:[unarchiver decodeBOOLWithName:@"isEditable"]];
  [self setSelectable:[unarchiver decodeBOOLWithName:@"isSelectable"]];
  [self setScrollable:[unarchiver decodeBOOLWithName:@"isScrollable"]];
  [self setContinuous:[unarchiver decodeBOOLWithName:@"isContinuous"]];
  /* Temporary commented out so buttons keep on working - new code
   * fixing this under testing */
  //    [self sendActionOn:[unarchiver decodeIntWithName:@"sendActionMask"]];
  
  return self;
}

@end /* NSCell (GMArchiverMethods) */



@implementation NSActionCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
    [super encodeWithModelArchiver:archiver];

    [archiver encodeInt:[self tag] withName:@"tag"];
    [archiver encodeObject:[self target] withName:@"target"];
    [archiver encodeSelector:[self action] withName:@"action"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
    self = [super initWithModelUnarchiver:unarchiver];

    // if (model_version >= 2) {
    [self setTag:[unarchiver decodeIntWithName:@"tag"]];
    [self setTarget:[unarchiver decodeObjectWithName:@"target"]];
    [self setAction:[unarchiver decodeSelectorWithName:@"action"]];
    // }

    return self;
}

@end /* NSActionCell (GMArchiverMethods) */


@implementation NSButtonCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
    float delay, interval;

    [super encodeWithModelArchiver:archiver];

    [self getPeriodicDelay:&delay interval:&interval];
    [archiver encodeFloat:delay withName:@"delay"];
    [archiver encodeFloat:interval withName:@"interval"];
    [archiver encodeString:[self title] withName:@"title"];
    [archiver encodeString:[self alternateTitle] withName:@"alternateTitle"];
    [archiver encodeObject:[self alternateImage] withName:@"alternateImage"];
    [archiver encodeInt:[self imagePosition] withName:@"imagePosition"];
    [archiver encodeBOOL:[self isTransparent] withName:@"isTransparent"];
    [archiver encodeString:[self keyEquivalent] withName:@"keyEquivalent"];
    [archiver encodeObject:[self keyEquivalentFont] withName:@"keyEquivalentFont"];
    [archiver encodeInt:[self keyEquivalentModifierMask] withName:@"keyEquivalentModifierMask"];
    [archiver encodeInt:[self highlightsBy] withName:@"highlightsBy"];
    [archiver encodeInt:[self showsStateBy] withName:@"showsStateBy"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
    float delay, interval;
    id obj;

    self = [super initWithModelUnarchiver:unarchiver];

    // if (model_version >= 2) {
    delay = [unarchiver decodeFloatWithName:@"delay"];
    interval = [unarchiver decodeFloatWithName:@"interval"];
    [self setPeriodicDelay:delay interval:interval];

    obj = [unarchiver decodeObjectWithName:@"alternateImage"];
    [self setAlternateImage:obj];
    [self setImagePosition:[unarchiver decodeIntWithName:@"imagePosition"]];
    [self setTransparent:[unarchiver decodeBOOLWithName:@"isTransparent"]];
    [self setKeyEquivalent:[unarchiver decodeStringWithName:@"keyEquivalent"]];
    [self setKeyEquivalentFont:[unarchiver decodeObjectWithName:@"keyEquivalentFont"]];
    [self setKeyEquivalentModifierMask:[unarchiver decodeIntWithName:@"keyEquivalentModifierMask"]];
    [self setHighlightsBy:[unarchiver decodeIntWithName:@"highlightsBy"]];
    [self setShowsStateBy:[unarchiver decodeIntWithName:@"showsStateBy"]];
    obj = [unarchiver decodeStringWithName:@"title"];
    if (obj) [self setTitle:obj];
    obj = [unarchiver decodeStringWithName:@"alternateTitle"];
    if (obj) [self setAlternateTitle:obj];
    // }

    return self;
}

@end /* NSButtonCell (GMArchiverMethods) */


@implementation NSMatrix (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
    [super encodeWithModelArchiver:archiver];

    [archiver encodeInt:[self mode] withName:@"mode"];
    [archiver encodeBOOL:[self allowsEmptySelection] withName:@"allowsEmptySelection"];
    [archiver encodeBOOL:[self isSelectionByRect] withName:@"isSelectionByRect"];

    [archiver encodeBOOL:[self autosizesCells] withName:@"autosizesCells"];
    [archiver encodeBOOL:[self isAutoscroll] withName:@"isAutoscroll"];
    [archiver encodeSize:[self cellSize] withName:@"cellSize"];
    [archiver encodeSize:[self intercellSpacing] withName:@"intercellSpacing"];
    [archiver encodeObject:[self backgroundColor] withName:@"backgroundColor"];
    [archiver encodeObject:[self cellBackgroundColor] withName:@"cellBackgroundColor"];
    [archiver encodeBOOL:[self drawsBackground] withName:@"drawsBackground"];
    [archiver encodeBOOL:[self drawsCellBackground] withName:@"drawsCellBackground"];

    [archiver encodeClass:[self cellClass] withName:@"cellClass"];
    [archiver encodeObject:[self prototype] withName:@"prototype"];
    [archiver encodeInt:[self numberOfRows] withName:@"numberOfRows"];
    [archiver encodeInt:[self numberOfColumns] withName:@"numberOfColumns"];
    [archiver encodeObject:[self cells] withName:@"cells"];
    [archiver encodeObject:[self delegate] withName:@"delegate"];

    [archiver encodeObject:[self target] withName:@"target"];
    [archiver encodeSelector:[self action] withName:@"action"];
    [archiver encodeSelector:[self doubleAction] withName:@"doubleAction"];
    [archiver encodeSelector:[self errorAction] withName:@"errorAction"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
    int nr, nc;
    NSArray *cell_array;
    int i;
    id decodedDelegate;
    
    self = [super initWithModelUnarchiver:unarchiver];

    // if (model_version >= 2) {
    [self setMode:[unarchiver decodeIntWithName:@"mode"]];
    [self setAllowsEmptySelection:[unarchiver decodeBOOLWithName:@"allowsEmptySelection"]];
    [self setSelectionByRect:[unarchiver decodeBOOLWithName:@"isSelectionByRect"]];

    [self setAutosizesCells:[unarchiver decodeBOOLWithName:@"autosizesCells"]];
    [self setAutoscroll:[unarchiver decodeBOOLWithName:@"isAutoscroll"]];
    [self setCellSize:[unarchiver decodeSizeWithName:@"cellSize"]];
    [self setIntercellSpacing:[unarchiver decodeSizeWithName:@"intercellSpacing"]];
    [self setBackgroundColor:[unarchiver decodeObjectWithName:@"backgroundColor"]];
    [self setCellBackgroundColor:[unarchiver decodeObjectWithName:@"cellBackgroundColor"]];
    [self setDrawsBackground:[unarchiver decodeBOOLWithName:@"drawsBackground"]];
    [self setDrawsCellBackground:[unarchiver decodeBOOLWithName:@"drawsCellBackground"]];

    [self setCellClass:[unarchiver decodeClassWithName:@"cellClass"]];
    [self setPrototype:[unarchiver decodeObjectWithName:@"prototype"]];
    
    nr = [unarchiver decodeIntWithName:@"numberOfRows"];
    nc = [unarchiver decodeIntWithName:@"numberOfColumns"];
    cell_array = [unarchiver decodeObjectWithName:@"cells"];
    [self renewRows:nr columns:nc];
#if GNU_GUI_LIBRARY
    _selectedRow = _selectedColumn = 0;
#endif
    for (i = 0; (i < [cell_array count]) && (i < nr*nc); i++)
    {
        id	cell = [cell_array objectAtIndex:i];

        [self putCell:cell atRow:i/nc column:i%nc];
        if ([cell state])
            [self selectCellAtRow:i/nc column:i%nc];
    }

    decodedDelegate = [unarchiver decodeObjectWithName:@"delegate"];
    if (decodedDelegate)
      [self setDelegate:decodedDelegate];


    [self setTarget:[unarchiver decodeObjectWithName:@"target"]];
    [self setAction:[unarchiver decodeSelectorWithName:@"action"]];
    [self setDoubleAction:[unarchiver decodeSelectorWithName:@"doubleAction"]];
    [self setErrorAction:[unarchiver decodeSelectorWithName:@"errorAction"]];
    [self sizeToCells];
    // }

    return self;
}

@end /* NSMatrix (GMArchiverMethods) */


@implementation NSScrollView (GMArchiverMethods)

// do not encode our subviews in NSView (it would encode the clipview and 
// the scroller, which are not necessary).
- (NSArray *)subviewsForModel
{
    return [NSArray array];
}

- (void) encodeWithModelArchiver: (GMArchiver*)archiver
{
  [super encodeWithModelArchiver: archiver];

  [archiver encodeObject: [self backgroundColor]
		withName: @"backgroundColor"];
  [archiver encodeInt: [self borderType]
	     withName: @"borderType"];
  [archiver encodeBOOL: [self hasHorizontalScroller]
	      withName: @"hasHorizontalScroller"];
  [archiver encodeBOOL: [self hasVerticalScroller]
	      withName: @"hasVerticalScroller"];
  [archiver encodeObject: [self documentView]
		withName: @"documentView"];
}

- (id) initWithModelUnarchiver: (GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver: unarchiver];

  [self setContentView: AUTORELEASE([NSClipView new])];

  [self setBorderType:
    [unarchiver decodeIntWithName: @"borderType"]];
  [self setHasHorizontalScroller:
    [unarchiver decodeBOOLWithName: @"hasHorizontalScroller"]];
  [self setHasVerticalScroller:
    [unarchiver decodeBOOLWithName: @"hasVerticalScroller"]];
  [self setDocumentView:
    [unarchiver decodeObjectWithName: @"documentView"]];
  [self setBackgroundColor:
    [unarchiver decodeObjectWithName: @"backgroundColor"]];

  return self;
}

@end /* NSScrollView (GMArchiverMethods) */


@implementation NSClipView (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeObject:[self documentView] withName:@"documentView"];
  [archiver encodeBOOL:[self copiesOnScroll] withName:@"copiesOnScroll"];
  if ([self respondsToSelector: @selector(drawsBackground)])
    [archiver encodeBOOL:[self drawsBackground] withName:@"drawsBackground"];
  [archiver encodeObject:[self backgroundColor] withName:@"backgroundColor"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setDocumentView:[unarchiver decodeObjectWithName:@"documentView"]];
  [self setCopiesOnScroll:[unarchiver decodeBOOLWithName:@"copiesOnScroll"]];
  if ([self respondsToSelector: @selector(setDrawsBackground:)])
    [self setDrawsBackground:[unarchiver decodeBOOLWithName:@"drawsBackground"]];
  [self setBackgroundColor:[unarchiver decodeObjectWithName:@"backgroundColor"]];
  return self;
}

@end /* NSClipView (GMArchiverMethods) */


@implementation NSColor (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  NSString* colorSpaceName = [self colorSpaceName];

  [archiver encodeString:colorSpaceName withName:@"colorSpaceName"];

  if ([colorSpaceName isEqual:@"NSDeviceCMYKColorSpace"]) {
    [archiver encodeFloat:[self cyanComponent] withName:@"cyan"];
    [archiver encodeFloat:[self magentaComponent] withName:@"magenta"];
    [archiver encodeFloat:[self yellowComponent] withName:@"yellow"];
    [archiver encodeFloat:[self blackComponent] withName:@"black"];
    [archiver encodeFloat:[self alphaComponent] withName:@"alpha"];
  }
  else if ([colorSpaceName isEqual:@"NSDeviceWhiteColorSpace"]
	   || [colorSpaceName isEqual:@"NSCalibratedWhiteColorSpace"]) {
    [archiver encodeFloat:[self whiteComponent] withName:@"white"];
    [archiver encodeFloat:[self alphaComponent] withName:@"alpha"];
  }
  else if ([colorSpaceName isEqual:@"NSDeviceRGBColorSpace"]
	   || [colorSpaceName isEqual:@"NSCalibratedRGBColorSpace"]) {
    [archiver encodeFloat:[self redComponent] withName:@"red"];
    [archiver encodeFloat:[self greenComponent] withName:@"green"];
    [archiver encodeFloat:[self blueComponent] withName:@"blue"];
    [archiver encodeFloat:[self alphaComponent] withName:@"alpha"];
    [archiver encodeFloat:[self hueComponent] withName:@"hue"];
    [archiver encodeFloat:[self saturationComponent] withName:@"saturation"];
    [archiver encodeFloat:[self brightnessComponent] withName:@"brightness"];
  }
  else if ([colorSpaceName isEqual:@"NSNamedColorSpace"]) {
    [archiver encodeString:[self catalogNameComponent]
		withName:@"catalogName"];
    [archiver encodeString:[self colorNameComponent] withName:@"colorName"];
  }
  else if ([colorSpaceName isEqual:@"NSPatternColorSpace"]) {
    [archiver encodeObject: [self patternImage] withName: @"patternImage"];
  }
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSString* colorSpaceName
      = [unarchiver decodeStringWithName:@"colorSpaceName"];

  if ([colorSpaceName isEqual:@"NSDeviceCMYKColorSpace"]) {
    float cyan = [unarchiver decodeFloatWithName:@"cyan"];
    float magenta = [unarchiver decodeFloatWithName:@"magenta"];
    float yellow = [unarchiver decodeFloatWithName:@"yellow"];
    float black = [unarchiver decodeFloatWithName:@"black"];
    float alpha = [unarchiver decodeFloatWithName:@"alpha"];

    return [NSColor colorWithDeviceCyan:cyan
				magenta:magenta
				 yellow:yellow
				  black:black
				  alpha:alpha];
  }
  else if ([colorSpaceName isEqual:@"NSDeviceWhiteColorSpace"]) {
    float white = [unarchiver decodeFloatWithName:@"white"];
    float alpha = [unarchiver decodeFloatWithName:@"alpha"];

    return [NSColor colorWithDeviceWhite:white alpha:alpha];
  }
  else if ([colorSpaceName isEqual:@"NSCalibratedWhiteColorSpace"]) {
    float white = [unarchiver decodeFloatWithName:@"white"];
    float alpha = [unarchiver decodeFloatWithName:@"alpha"];

    return [NSColor colorWithCalibratedWhite:white alpha:alpha];
  }
  else if ([colorSpaceName isEqual:@"NSDeviceRGBColorSpace"]) {
    float red = [unarchiver decodeFloatWithName:@"red"];
    float green = [unarchiver decodeFloatWithName:@"green"];
    float blue = [unarchiver decodeFloatWithName:@"blue"];
    float alpha = [unarchiver decodeFloatWithName:@"alpha"];

    return [self colorWithDeviceRed:red green:green blue:blue alpha:alpha];
  }
  else if ([colorSpaceName isEqual:@"NSCalibratedRGBColorSpace"]) {
    float red = [unarchiver decodeFloatWithName:@"red"];
    float green = [unarchiver decodeFloatWithName:@"green"];
    float blue = [unarchiver decodeFloatWithName:@"blue"];
    float alpha = [unarchiver decodeFloatWithName:@"alpha"];

    return [self colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
  }
  else if ([colorSpaceName isEqual:@"NSNamedColorSpace"]) {
    NSString *catalog = [unarchiver decodeStringWithName: @"catalogName"];
    NSString *colornm = [unarchiver decodeStringWithName: @"colorName"];
    return [self colorWithCatalogName: catalog colorName: colornm];
  }
  else if ([colorSpaceName isEqual:@"NSPatternColorSpace"]) {
    NSImage *image = [unarchiver decodeObjectWithName: @"patternImage"];
    if (image == nil)
      {
	NSLog(@"Internal: No can't decode colorspace %@", colorSpaceName);
	NSLog(@"          creating generic white color");
	return [NSColor colorWithDeviceWhite: 1.0  alpha: 1.0];
      }
    else
      return [NSColor colorWithPatternImage: image];
  }
  else
    {
      NSLog(@"Internal: No decoder for colorspace %@", colorSpaceName);
      NSLog(@"          creating generic white color");
      return [NSColor colorWithDeviceWhite: 1.0  alpha: 1.0];
    }
  return nil;
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return self;
}

- (Class)classForModelArchiver
{
  return [NSColor class];
}

@end /* NSColor (GMArchiverMethods) */


@implementation NSControl (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
    [archiver encodeObject:[self cell] withName:@"cell"];
    [archiver encodeBOOL:[self isEnabled] withName:@"isEnabled"];
    [archiver encodeInt:[self tag] withName:@"tag"];
    [archiver encodeBOOL:[self ignoresMultiClick] withName:@"ignoresMultiClick"];

    [super encodeWithModelArchiver:archiver];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
    id decodedCell;
  
    self = [super initWithModelUnarchiver:unarchiver];

    // if (model_version == 1) {
    //[self setTarget:[unarchiver decodeObjectWithName:@"target"]];
    //[self setAction:[unarchiver decodeSelectorWithName:@"action"]];
    //[self setEnabled:[unarchiver decodeBOOLWithName:@"isEnabled"]];
    //[self setAlignment:[unarchiver decodeIntWithName:@"alignment"]];
    //[self setFont:[unarchiver decodeObjectWithName:@"font"]];
    //[self setContinuous:[unarchiver decodeBOOLWithName:@"isContinuous"]];
    //[self setTag:[unarchiver decodeIntWithName:@"tag"]];
    //[self setIgnoresMultiClick:
    //            [unarchiver decodeBOOLWithName:@"ignoresMultiClick"]];
    // } else {
    {
      // So that custom NSControls, which do not encode the cell, 
      // can still work.
      decodedCell = [unarchiver decodeObjectWithName:@"cell"];
      if (decodedCell)
	[self setCell: decodedCell];
      else
	[self setCell: AUTORELEASE([[[self class] cellClass] new])];
    }
    [self setEnabled:[unarchiver decodeBOOLWithName:@"isEnabled"]];
    [self setTag:[unarchiver decodeIntWithName:@"tag"]];
    [self setIgnoresMultiClick:
                [unarchiver decodeBOOLWithName:@"ignoresMultiClick"]];
    // }
    return self;
}

@end /* NSControl (GMArchiverMethods) */

@implementation NSFont (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeString:[self fontName] withName:@"name"];
  [archiver encodeFloat:[self pointSize] withName:@"size"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSFont *f = [NSFont fontWithName:[unarchiver decodeStringWithName:@"name"]
		 size:[unarchiver decodeFloatWithName:@"size"]];
  if (!f)
    f = [NSFont systemFontOfSize: [unarchiver decodeFloatWithName:@"size"]];
  return f;
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return self;
}

@end /* NSFont (GMArchiverMethods) */


@implementation NSImage (GMArchiverMethods)

extern id _nibOwner;

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeString:[self name] withName:@"name"];
  [archiver encodeSize:[self size] withName:@"size"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  id image = nil;
  NSString *imageName;

  imageName = [unarchiver decodeStringWithName:@"name"];

  if (imageName)
  {
    image = [NSImage imageNamed: imageName];
    if (image == nil)
    {
       NSBundle *bundle = [NSBundle bundleForClass:[_nibOwner class]];
       NSString *path = [bundle pathForImageResource:imageName];

       image = [[NSImage alloc] initByReferencingFile:path];
    }
  }

  if (image == nil)
    image = [NSImage imageNamed:@"GNUstepMenuImage"];

  return image;
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  [self setSize:[unarchiver decodeSizeWithName:@"size"]];
  return self;
}

@end /* NSImage (GMArchiverMethods) */


@implementation NSMenuItem (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeString:[self title] withName:@"title"];
  if ([self hasSubmenu] == NO)
    {
      if ([self respondsToSelector: @selector(image)])
        [archiver encodeObject:[self image] withName:@"image"];
      if ([self respondsToSelector: @selector(onStateImage)])
        [archiver encodeObject:[self onStateImage] withName:@"onStateImage"];
      if ([self respondsToSelector: @selector(offStateImage)])
        [archiver encodeObject:[self offStateImage] withName:@"offStateImage"];
      if ([self respondsToSelector: @selector(mixedStateImage)])
         [archiver encodeObject:[self mixedStateImage]
                       withName:@"mixedStateImage"];
    }
  [archiver encodeString:[self keyEquivalent] withName:@"keyEquivalent"];
  if ([self respondsToSelector: @selector(state)])
    {
      [archiver encodeInt:[self state] withName:@"state"];
    }
  [archiver encodeObject:[self target] withName:@"target"];
  [archiver encodeSelector:[self action] withName:@"action"];
  [archiver encodeInt:[self tag] withName:@"tag"];
  [archiver encodeBOOL:[self isEnabled] withName:@"isEnabled"];
  if ([self respondsToSelector: @selector(changesState)])
    {
      [archiver encodeBOOL:[self changesState] withName:@"changesState"];
    }
  if ([self respondsToSelector: @selector(submenu)])
    {
      [archiver encodeObject:[self submenu] withName:@"submenu"];
    }
  [archiver encodeConditionalObject:[self representedObject]
	                   withName:@"representedObject"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  [self setTitle:[unarchiver decodeStringWithName:@"title"]];
  [self setImage:[unarchiver decodeObjectWithName:@"image"]];
  [self setOnStateImage:[unarchiver decodeObjectWithName:@"onStateImage"]];
  [self setOffStateImage:[unarchiver decodeObjectWithName:@"offStateImage"]];
  [self setMixedStateImage:[unarchiver
			     decodeObjectWithName:@"mixedStateImage"]];
  [self setKeyEquivalent:[unarchiver decodeStringWithName:@"keyEquivalent"]];
  [self setState:[unarchiver decodeIntWithName:@"state"]];
  [self setAction:[unarchiver decodeSelectorWithName:@"action"]];
  [self setTarget: [unarchiver decodeObjectWithName: @"target"]];
  [self setTag:[unarchiver decodeIntWithName:@"tag"]];
  [self setEnabled:[unarchiver decodeBOOLWithName:@"isEnabled"]];
  [self setChangesState:[unarchiver decodeBOOLWithName:@"changesState"]];
  [self setRepresentedObject:[unarchiver
			       decodeObjectWithName:@"representedObject"]];

  /* 
   * Here we have quite a delicate point. 
   * In OPENSTEP 4.x, NSMenuItems with submenus have their submenu 
   * as target; they do not answer to -submenu, so the encoded submenu 
   * is nil; the submenu is actually encoded as the target.
   *
   * In GNUstep the target is instead the menu, and the submenu 
   * is stored in an additional ivar.
   *
   * I don't know about MacOS-X.
   *
   * This code needs to be able to decode gmodels created on OPENSTEP 4.x too,
   * that's why we have the following.
   */

  /* Safety assignment. */
  [self setMenu: nil];

  /* Set submenu ... but not if it's nil, because this assignment
   * always has side effects on action and target and we don't want to mess
   * them up if we don't have a submenu!  */
  {
    id submenu = [unarchiver decodeObjectWithName: @"submenu"];
    
    if (submenu != nil)
      {
	[self setSubmenu: submenu];
      }
  }
  /* Now we fix the submenu from the target if needed: */
#ifdef GNU_GUI_LIBRARY
  /*
   * Set submenu from target if not set (this happens if the gmodel 
   * was created on OPENSTEP).
   */
  if ([NSStringFromSelector ([self action]) 
			    isEqualToString: @"submenuAction:"])
    {
      if ([self submenu] == nil)
	{
	  if ([[self target] isKindOfClass: [NSMenu class]])
	    {
	      [self setSubmenu: [self target]];
	    }
	  else
	    {
	      NSLog(@"Error decoding gmodel - submenu not an NSMenu");
	    }
	}
    }
#endif

  /* NB: The target might now be wrong (in case the gmodel was encoded
     in a platform where the target is the submenu while we now want
     the target to be the menu) !  This is automatically fixed later,
     because after the NSMenuItem is decoded, it is added to its
     container NSMenu, which will fix the target. */

#if 0
  NSLog (@"menu item %@: target = %@, isEnabled = %d",
	[self title], [self target], [self isEnabled]);
#endif

  return self;
}

@end /* NSMenuItem (GMArchiverMethods) */


@implementation NSMenu (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeString:[self title] withName:@"title"];
  [archiver encodeObject:[self itemArray] withName:@"itemArray"];
  [archiver encodeBOOL:[self autoenablesItems] withName:@"autoenablesItems"];
}

/* Define this method here because on OPENSTEP 4.x the NSMenu is inherited from
   NSWindow and we don't want the NSWindow's method to be called. */
+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSString* theTitle = [unarchiver decodeStringWithName:@"title"];
  return [[[self allocWithZone:[unarchiver objectZone]] initWithTitle:theTitle]
		autorelease];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  int i, count;
  NSMutableArray* decodedItems;

  decodedItems = [unarchiver decodeObjectWithName: @"itemArray"];
  count = [decodedItems count];

  for (i = 0; i < count; i++)
    {
      id item = [decodedItems objectAtIndex: i];
      
      [self addItem: item];
    }

  [self setAutoenablesItems: [unarchiver decodeBOOLWithName: 
					   @"autoenablesItems"]];
  [self sizeToFit];

  return self;
}

@end /* NSMenu (GMArchiverMethods) */

/* This class is special - to avoid having the cell encoded we encode 
   it directly.  Perhaps it would be wiser to do differently. */
@implementation NSPopUpButton (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [archiver encodeRect:     [self frame]     withName: @"frame"];
  [archiver encodeBOOL:     [self pullsDown] withName: @"pullsDown"];
  [archiver encodeBOOL:     [self isEnabled] withName: @"isEnabled"];
  [archiver encodeInt:      [self tag]       withName: @"tag"];
  [archiver encodeObject:   [self target]    withName: @"target"];
  [archiver encodeSelector: [self action]    withName: @"action"];
  [archiver encodeInt:      [self autoresizingMask]
             withName:      @"autoresizingMask"];
  
#if 0 //def __APPLE__
  {
    int i;
    NSMutableArray *array;
    array = [NSMutableArray arrayWithCapacity: [self numberOfItems]];
    for (i = 0; i < [self numberOfItems]; i++)
      [array addObject: [self itemAtIndex: i]];
    [archiver encodeArray: array withName: @"itemArray"];
  }
#else
  [archiver encodeArray:[self itemArray] withName:@"itemArray"];
#endif

  [archiver encodeString:[[self selectedItem] title] withName:@"selectedItem"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSRect         rect;
  NSPopUpButton *popup;
  BOOL           pullsDown;

  rect      = [unarchiver decodeRectWithName: @"frame"];
  pullsDown = [unarchiver decodeBOOLWithName: @"pullsDown"];

  popup = [NSPopUpButton allocWithZone: [unarchiver objectZone]];
  popup = [popup initWithFrame: rect pullsDown: pullsDown];
  AUTORELEASE (popup);
  
  return popup;
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  int      i, count;
  NSString *string;
  NSArray  *decodedItems;
  
  decodedItems = [unarchiver decodeArrayWithName: @"itemArray"];
  
  if (decodedItems && [decodedItems count]) 
    {
      count = [decodedItems count]; 
      for (i = 0; i < count; i++) 
	{
	  id item = [decodedItems objectAtIndex:i];
	  id myItem;
      
	  [self addItemWithTitle: [item title]];
	  myItem = [self itemAtIndex:i];
	  [myItem setTarget:        [item target]];
	  [myItem setAction:        [item action]];
	  [myItem setEnabled:       [item isEnabled]];
	  [myItem setTag:           [item tag]];
	  [myItem setKeyEquivalent: [item keyEquivalent]];
	}
      string = [unarchiver decodeStringWithName: @"selectedItem"];
      [self selectItemWithTitle: string];
    }
  else
    {
      /* For old gmodels that didn't support popups */
      [self addItemWithTitle: @"Item 1"];
      [self selectItemAtIndex: 0];
    }

  [self setEnabled: [unarchiver decodeBOOLWithName:     @"isEnabled"]];
  [self setTag:     [unarchiver decodeIntWithName:      @"tag"]];
  [self setTarget:  [unarchiver decodeObjectWithName:   @"target"]];
  [self setAction:  [unarchiver decodeSelectorWithName: @"action"]];

  [self setAutoresizingMask:
      [unarchiver decodeIntWithName: @"autoresizingMask"]];
  
  return self;
}

@end /* NSPopUpButton (GMArchiverMethods) */

@implementation NSResponder (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  id nextResponder;

  if ((nextResponder = [self nextResponder]))
    [archiver encodeObject:nextResponder withName:@"nextResponder"];
  if ([self respondsToSelector: @selector(interfaceStyle)])
    [archiver encodeUnsignedInt: [self interfaceStyle]
            withName:@"interfaceStyle"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  [self setNextResponder:[unarchiver decodeObjectWithName:@"nextResponder"]];
  if ([self respondsToSelector: @selector(setInterfaceStyle:)])
    [self setInterfaceStyle:
        [unarchiver decodeUnsignedIntWithName:@"interfaceStyle"]];

  return self;
}

@end /* NSResponder (GMArchiverMethods) */

@implementation NSTextField (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeSelector:[self errorAction] withName:@"errorAction"];
  [archiver encodeObject:[self delegate] withName:@"delegate"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setErrorAction:[unarchiver decodeSelectorWithName:@"errorAction"]];
  [self setDelegate:[unarchiver decodeObjectWithName:@"delegate"]];

  return self;
}

@end /* NSTextField (GMArchiverMethods) */

@implementation NSSecureTextFieldCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];
  if ([self respondsToSelector: @selector(echosBullets)])
    [archiver encodeBOOL:[self echosBullets] withName:@"echosBullets"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];
  if ([self respondsToSelector: @selector(setEchosBullets:)])
    [self setEchosBullets:[unarchiver decodeBOOLWithName:@"echosBullets"]];

  return self;
}

@end /* NSSecureTextFieldCell (GMArchiverMethods) */

@implementation NSView (GMArchiverMethods)

// subclasses may not want to encode all subviews...
- (NSArray *)subviewsForModel
{
#ifdef GNU_GUI_LIBRARY  
  return [self subviews];
#else /* Do not encode Apple's private classes */
  NSArray        *views;
  NSMutableArray *viewsToEncode;
  id             e, o;
  NSString       *classString;
  
  views = [self subviews];
  viewsToEncode = [NSMutableArray array];
  e = [views objectEnumerator];
  while ((o = [e nextObject]))
    {
      classString = NSStringFromClass ([o class]);
      if ([classString isEqualToString: @"_NSKeyboardFocusClipView"] == NO)
	[viewsToEncode addObject: o];
    }
  
  return viewsToEncode;
#endif
}

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeConditionalObject:[self superview] withName:@"superview"];
  [archiver encodeObject:[self subviewsForModel] withName:@"subviews"];
  [archiver encodeRect:[self frame] withName:@"frame"];
  [archiver encodeRect:[self bounds] withName:@"bounds"];
  [archiver encodeBOOL:[self postsFrameChangedNotifications]
	    withName:@"postsFrameChangedNotifications"];
  [archiver encodeBOOL:[self postsBoundsChangedNotifications]
	    withName:@"postsBoundsChangedNotifications"];
  [archiver encodeBOOL:[self autoresizesSubviews]
	    withName:@"autoresizesSubviews"];
  [archiver encodeUnsignedInt:[self autoresizingMask]
	    withName:@"autoresizingMask"];
  [archiver encodeConditionalObject:[self nextKeyView] withName:@"nextKeyView"];
  [archiver encodeConditionalObject:[self previousKeyView] 
	    withName:@"previousKeyView"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSRect rect = [unarchiver decodeRectWithName:@"frame"];
  NSView* view = [[[self allocWithZone:[unarchiver objectZone]]
				initWithFrame:rect]
				autorelease];
  if (!view)
    NSLog (@"cannot create the requested view!");
  return view;
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSArray* subviews;
  int i, count;
  id superview;

  self = [super initWithModelUnarchiver:unarchiver];

  superview = [unarchiver decodeObjectWithName:@"superview"];
  /* If we are the content view of an NSBox, don't add ourselves to it.
     This is done later in the NSBox unachiver method */
  if ([superview isKindOfClass: [NSBox class]] == NO)
    [superview addSubview:self];

  subviews = [unarchiver decodeObjectWithName:@"subviews"];
  for (i = 0, count = [subviews count]; i < count; i++)
    [self addSubview:[subviews objectAtIndex:i]];

//  [self setBounds:[unarchiver decodeRectWithName:@"bounds"]];
  [self setPostsFrameChangedNotifications:
	[unarchiver decodeBOOLWithName:@"postsFrameChangedNotifications"]];
  [self setPostsBoundsChangedNotifications:
	[unarchiver decodeBOOLWithName:@"postsBoundsChangedNotifications"]];
  [self setAutoresizesSubviews:
	[unarchiver decodeBOOLWithName:@"autoresizesSubviews"]];
  [self setAutoresizingMask:
	[unarchiver decodeUnsignedIntWithName:@"autoresizingMask"]];
  [self setNextKeyView: [unarchiver decodeObjectWithName:@"nextKeyView"]];
  [self setPreviousKeyView: 
	  [unarchiver decodeObjectWithName:@"previousKeyView"]];


#ifdef GNU_GUI_LIBRARY
  _rFlags.flipped_view = [self isFlipped];
  if ([_sub_views count])
    _rFlags.has_subviews = 1;
#endif

  return self;
}

@end /* NSView (GMArchiverMethods) */


@implementation NSWindow (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  NSPoint wnOrigin = [self frame].origin;
  NSRect ctFrame = [[self contentView] frame], minRect;
  unsigned int style;

  ctFrame.origin = wnOrigin;

  /* convert minSize to GNUstep frame (without title bar and resize bar) */
  minRect.origin = wnOrigin;
  minRect.size = [self minSize];
  minRect = [NSWindow contentRectForFrameRect:minRect
                                    styleMask:[self styleMask]];

  [archiver encodeRect:ctFrame withName:@"contentFrame"];
  [archiver encodeSize:[self maxSize] withName:@"maxSize"];
  [archiver encodeSize:minRect.size withName:@"minSize"];
  [archiver encodeString:[self frameAutosaveName]
	    withName:@"frameAutosaveName"];
  [archiver encodeInt:[self level] withName:@"level"];
  [archiver encodeBOOL:[self isVisible] withName:@"isVisible"];
  [archiver encodeBOOL:[self isAutodisplay] withName:@"isAutodisplay"];
  [archiver encodeString:[self title] withName:@"title"];
  [archiver encodeString:[self representedFilename]
	    withName:@"representedFilename"];
  [archiver encodeBOOL:[self isReleasedWhenClosed]
	    withName:@"isReleasedWhenClosed"];
  [archiver encodeObject:[self contentView] withName:@"contentView"];
  [archiver encodeBOOL:[self hidesOnDeactivate]
	    withName:@"hidesOnDeactivate"];
  [archiver encodeObject:[self backgroundColor] withName:@"backgroundColor"];

  style = [self styleMask];
#ifndef GNU_GUI_LIBRARY
  /* Work around a bug in OpenStep, which doesn't set the
   * NSTitledWindowMask properly.  If the window is not borderless,
   * always add the title mask.  */
  if (style != NSBorderlessWindowMask)
    {
      style |= NSTitledWindowMask;
    }
#endif

  [archiver encodeUnsignedInt: style  withName: @"styleMask"];
  [archiver encodeUnsignedInt: [self backingType]  withName: @"backingType"];
  [archiver encodeConditionalObject: [self initialFirstResponder] 
	    withName: @"initialFirstResponder"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  unsigned backingType = [unarchiver decodeUnsignedIntWithName:@"backingType"];
  unsigned styleMask = [unarchiver decodeUnsignedIntWithName:@"styleMask"];
  NSRect ctRect = [unarchiver decodeRectWithName:@"contentFrame"];

  NSWindow	*win = [[self allocWithZone: [unarchiver objectZone]]
			initWithContentRect: ctRect
				  styleMask: styleMask
				    backing: backingType
				      defer: YES];
  //  printf("content: %g, %g -- frame %g, %g\n", ctRect.size.width, ctRect.size.height, [win frame].size.width, [win frame].size.height);

  return AUTORELEASE(win);
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  NSString* frameAutosaveName;

  [self setContentView:[unarchiver decodeObjectWithName:@"contentView"]];
  [self setMaxSize:[unarchiver decodeSizeWithName:@"maxSize"]];
  [self setMinSize:[unarchiver decodeSizeWithName:@"minSize"]];

  frameAutosaveName = [unarchiver decodeStringWithName:@"frameAutosaveName"];
  if (frameAutosaveName)
    [self setFrameAutosaveName:frameAutosaveName];

#ifdef GNU_GUI_LIBRARY
  _windowLevel = [unarchiver decodeIntWithName:@"level"];
#endif

  [self setInitialFirstResponder: 
	  [unarchiver decodeObjectWithName:@"initialFirstResponder"]];
  [self setAutodisplay:[unarchiver decodeBOOLWithName:@"isAutodisplay"]];
  [self setTitle:[unarchiver decodeStringWithName:@"title"]];
  [self setRepresentedFilename:
	  [unarchiver decodeStringWithName:@"representedFilename"]];
  [self setReleasedWhenClosed:
	  [unarchiver decodeBOOLWithName:@"isReleasedWhenClosed"]];
  [self setHidesOnDeactivate:
	  [unarchiver decodeBOOLWithName:@"hidesOnDeactivate"]];
  [self setBackgroundColor:
	  [unarchiver decodeObjectWithName:@"backgroundColor"]];
  if ([unarchiver decodeBOOLWithName:@"isVisible"])
    {
      /* If we are the main gmodel of the application, this code is
	 being executed before the app became active, so we have to
	 force ordering front of the window.  TODO: Refine the code so
	 that if we are not the main gmodel then a more appropriate 
         [self orderFront:nil] is used.*/
      [self orderFrontRegardless];
    }

#if GNU_GUI_LIBRARY
	[[self contentView] setNeedsDisplay:YES];
#endif

  return self;
}

@end /* NSWindow (GMArchiverMethods) */

@implementation NSPanel (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
    [super encodeWithModelArchiver: archiver];
    
    [archiver encodeBOOL:[self isFloatingPanel] withName:@"isFloatingPanel"];
    [archiver encodeBOOL:[self becomesKeyOnlyIfNeeded]
	      withName:@"becomesKeyOnlyIfNeeded"];
    [archiver encodeBOOL:[self worksWhenModal] withName:@"worksWhenModal"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
    unsigned backingType = [unarchiver decodeUnsignedIntWithName:
			   @"backingType"];
    unsigned styleMask = [unarchiver decodeUnsignedIntWithName:@"styleMask"];
    NSRect ctRect = [unarchiver decodeRectWithName:@"contentFrame"];
    NSPanel* panel = [[[[self class] allocWithZone:[unarchiver objectZone]]
		     initWithContentRect:ctRect
		     styleMask:styleMask backing:backingType defer:YES]
		    autorelease];
    // use [self class] here, so it works for subclasses

    return panel;
}

-(id)initWithModelUnarchiver :(GMUnarchiver *)unarchiver
{
    [super initWithModelUnarchiver: unarchiver];

    [self setFloatingPanel: 
	    [unarchiver decodeBOOLWithName: @"isFloatingPanel"]];
    [self setBecomesKeyOnlyIfNeeded:
	    [unarchiver decodeBOOLWithName: @"becomesKeyOnlyIfNeeded"]];
    [self setWorksWhenModal:
	    [unarchiver decodeBOOLWithName: @"setWorksWhenModal"]];

    return self;
}

@end  /* NSPanel (GMArchiverMethods) */


@implementation NSSavePanel (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver: archiver];

  [archiver encodeString:[self prompt] withName:@"prompt"];
  [archiver encodeObject:[self accessoryView] withName:@"accessoryView"];
  [archiver encodeString:[self requiredFileType]
            withName: @"requiredFileType"];
  [archiver encodeBOOL:[self treatsFilePackagesAsDirectories]
            withName: @"treatsFilePackagesAsDirectories"];
  [archiver encodeString:[self directory]
            withName: @"directory"];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
    unsigned backingType = [unarchiver decodeUnsignedIntWithName:
			   @"backingType"];
    unsigned styleMask = [unarchiver decodeUnsignedIntWithName:@"styleMask"];
    NSRect aRect = [unarchiver decodeRectWithName:@"frame"];
    // Use [self class] here instead of NSSavePanel so as to invoke
    // +allocWithZone on the correct (if any) sub-class
    NSSavePanel* panel = [[[[self class] allocWithZone:[unarchiver objectZone]]
			  initWithContentRect:aRect
			  styleMask:styleMask backing:backingType defer:YES]
			 autorelease];

#if GNU_GUI_LIBRARY
    NSDebugLLog(@"NSSavePanel", @"NSSavePanel +createObjectForModelUnarchiver");
#endif
    return panel;
}

-(id)initWithModelUnarchiver :(GMUnarchiver *)unarchiver
{
    [super initWithModelUnarchiver: unarchiver];

    //NSSavePanel specifics
    [self setPrompt:[unarchiver decodeStringWithName:@"prompt"]];
    [self setAccessoryView:[unarchiver decodeObjectWithName:@"accessoryView"]];
    [self setRequiredFileType:
          [unarchiver decodeStringWithName:@"requiredFileType"]];
    [self setTreatsFilePackagesAsDirectories:
          [unarchiver decodeBOOLWithName:@"treatsFilePackagesAsDirectories"]];
    [self setDirectory:
          [unarchiver decodeStringWithName:@"directory"]];

#if GNU_GUI_LIBRARY
    [[self contentView] setNeedsDisplay:YES];
#endif
    return self;
}

@end  /* NSSavePanel (GMArchiverMethods) */

@implementation NSOpenPanel (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:(GMArchiver*)archiver];
  [archiver encodeBOOL:[self canChooseFiles] 
	    withName:@"canChooseFiles"];
  [archiver encodeBOOL:[self canChooseDirectories] 
	    withName:@"canChooseDirectories"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver *)unarchiver
{
  [super initWithModelUnarchiver:(GMUnarchiver *)unarchiver];
  [self setCanChooseFiles:[unarchiver decodeBOOLWithName:@"canChooseFiles"]];
  [self setCanChooseDirectories:[unarchiver decodeBOOLWithName:
					      @"canChooseDirectories"]];
  return self;
}
@end  /* NSOpenPanel (GMArchiverMethods) */

@implementation NSBrowser (GMArchiverMethods)

- (void)encodeWithModelArchiver :(GMArchiver*)archiver
{
    [super encodeWithModelArchiver:archiver];

    //NSBrowser
    [archiver encodeString:[self path] withName:@"path"];
    [archiver encodeString:[self pathSeparator] withName:@"pathSeparator"];
    [archiver encodeBOOL:[self allowsBranchSelection] 
            withName:@"allowsBranchSelection"];
    [archiver encodeBOOL:[self allowsEmptySelection]
            withName:@"allowsEmptySelection"];
    [archiver encodeBOOL:[self allowsMultipleSelection]
            withName:@"allowsMultipleSelection"];
    [archiver encodeBOOL:[self reusesColumns] withName:@"reusesColumns"];
    [archiver encodeUnsignedInt:[self maxVisibleColumns]
            withName:@"maxVisibleColumns"];
    [archiver encodeUnsignedInt:[self minColumnWidth]
            withName:@"minColumnWidth"];
    [archiver encodeBOOL:[self separatesColumns]
            withName:@"separatesColumns"];
    [archiver encodeBOOL:[self takesTitleFromPreviousColumn]
            withName:@"takesTitleFromPreviousColumn"];
    [archiver encodeBOOL:[self isTitled] withName:@"isTitled"];
    [archiver encodeBOOL:[self hasHorizontalScroller]
            withName:@"hasHorizontalScroller"];
    [archiver encodeBOOL:[self acceptsArrowKeys]
            withName:@"acceptsArrowKeys"];
    [archiver encodeBOOL:[self sendsActionOnArrowKeys]
            withName:@"sendsActionOnArrowKeys"];

    [archiver encodeObject:[self delegate] withName:@"delegate"];
    [archiver encodeSelector:[self doubleAction] withName:@"doubleAction"];
}

#if 0
+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
    unsigned backingType = [unarchiver decodeUnsignedIntWithName:
			   @"backingType"];
    unsigned styleMask = [unarchiver decodeUnsignedIntWithName:@"styleMask"];
    NSRect aRect = [unarchiver decodeRectWithName:@"frame"];
    NSBrowser* browser = [[[NSBrowser allocWithZone:[unarchiver objectZone]]
			  initWithContentRect:aRect
			  styleMask:styleMask backing:backingType defer:YES]
			 autorelease];

    return browser;
}
#endif

- (id)initWithModelUnarchiver :(GMUnarchiver *)unarchiver
{
    id delegate;


    self = [super initWithModelUnarchiver:unarchiver];
    
    [self setPath:[unarchiver decodeStringWithName:@"path"]];
    [self setPathSeparator:[unarchiver decodeStringWithName:@"pathSeparator"]];
    [self setAllowsBranchSelection:[unarchiver
		       decodeBOOLWithName:@"allowsBranchSelection"]];
    [self setAllowsEmptySelection:[unarchiver
		       decodeBOOLWithName:@"allowsEmptySelection"]];
    [self setAllowsMultipleSelection:[unarchiver
		       decodeBOOLWithName:@"allowsMultipleSelection"]];

    [self setReusesColumns:[unarchiver decodeBOOLWithName:@"reusesColumns"]];
    [self setMaxVisibleColumns:[unarchiver
		       decodeUnsignedIntWithName:@"maxVisibleColumns"]];
    [self setMinColumnWidth:[unarchiver
		       decodeUnsignedIntWithName:@"minColumnWidth"]];
    [self setSeparatesColumns:[unarchiver
		       decodeBOOLWithName:@"separatesColumns"]];
    [self setTakesTitleFromPreviousColumn:[unarchiver 
		       decodeBOOLWithName:@"takesTitleFromPreviousColumn"]];
    [self setTitled:[unarchiver 
		       decodeBOOLWithName:@"isTitled"]];
    [self setHasHorizontalScroller:[unarchiver
		       decodeBOOLWithName:@"hasHorizontalScroller"]];
    [self setAcceptsArrowKeys:[unarchiver 
                       decodeBOOLWithName:@"acceptsArrowKeys"]];
    [self setSendsActionOnArrowKeys:[unarchiver
		       decodeBOOLWithName:@"sendsActionOnArrowKeys"]];

    //avoid an exeption
    delegate = [unarchiver decodeObjectWithName:@"delegate"];
    if (delegate)
	[self setDelegate:delegate];

    [self setDoubleAction:[unarchiver decodeSelectorWithName:@"doubleAction"]];
    return self;
}

@end  /* NSBrowser (GMArchiverMethods) */

@implementation NSColorWell (GMArchiverMethods)

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setColor:[unarchiver decodeObjectWithName:@"color"]];
  [self setBordered:[unarchiver decodeBOOLWithName:@"isBordered"]];

  return self;
}

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeObject:[self color] withName:@"color"];
  [archiver encodeBOOL:[self isBordered] withName:@"isBordered"];
}

@end /* NSColorWell (GMArchiverMethods) */

@implementation NSImageView (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeInt:[self imageAlignment] withName:@"alignment"];
  [archiver encodeInt:[self imageFrameStyle] withName:@"frameStyle"];
  [archiver encodeObject:[self image] withName:@"image"];
  [archiver encodeBOOL:[self isEditable] withName:@"isEditable"];
  [archiver encodeInt:[self imageScaling] withName:@"scaling"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setImageAlignment:[unarchiver decodeIntWithName:@"alignment"]];
  [self setImageFrameStyle:[unarchiver decodeIntWithName:@"frameStyle"]];
  [self setImage:[unarchiver decodeObjectWithName:@"image"]];
  [self setEditable:[unarchiver decodeBOOLWithName:@"isEditable"]];
  [self setImageScaling:[unarchiver decodeIntWithName:@"scaling"]];

  return self;
}

@end

@implementation NSSliderCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeFloat:[self knobThickness] withName:@"knobThickness"];
  [archiver encodeObject:[self image] withName:@"image"];
  [archiver encodeDouble:[self maxValue] withName:@"maxValue"];
  [archiver encodeDouble:[self minValue] withName:@"minValue"];

  // title, color, and font info is encoded by the title cell
  [archiver encodeObject:[self titleCell] withName:@"titleCell"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  [super initWithModelUnarchiver:unarchiver];

  [self setKnobThickness:[unarchiver decodeFloatWithName:@"knobThickness"]];
  [self setImage:[unarchiver decodeObjectWithName:@"image"]];
  [self setMaxValue:[unarchiver decodeDoubleWithName:@"maxValue"]];
  [self setMinValue:[unarchiver decodeDoubleWithName:@"minValue"]];

  // title, color, and font info is encoded by the title cell
  [self setTitleCell:[unarchiver decodeObjectWithName:@"titleCell"]];

  return self;
}

@end /* NSSliderCell (GMArchiverMethods) */

@implementation NSTextFieldCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeObject:[self backgroundColor] withName:@"backgroundColor"];
  [archiver encodeBOOL:[self drawsBackground] withName:@"drawsBackground"];
  [archiver encodeObject:[self textColor] withName:@"textColor"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setBackgroundColor:
	     [unarchiver decodeObjectWithName:@"backgroundColor"]];
  [self setDrawsBackground:
	     [unarchiver decodeBOOLWithName:@"drawsBackground"]];
  [self setTextColor:[unarchiver decodeObjectWithName:@"textColor"]];

  return self;
}

@end /* NSTextFieldCell (GMArchiverMethods) */

@implementation NSFormCell (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeString:[self title] withName:@"title"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver:unarchiver];

  [self setTitle:[unarchiver decodeStringWithName:@"title"]];

  return self;
}

@end /* NSFormCell (GMArchiverMethods) */

@implementation NSText (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  [archiver encodeString: [self string] withName: @"string"];
  [archiver encodeConditionalObject: [self backgroundColor] 
	    withName: @"backgroundColor"];
  [archiver encodeBOOL:[self drawsBackground] withName:@"drawsBackground"];
  [archiver encodeObject:[self textColor] withName:@"textColor"];
  [archiver encodeBOOL:[self isEditable] withName:@"isEditable"];
  [archiver encodeBOOL:[self isSelectable] withName:@"isSelectable"];
  [archiver encodeBOOL:[self isFieldEditor] withName:@"isFieldEditor"];
  [archiver encodeBOOL:[self isRichText] withName:@"isRichText"];
  [archiver encodeBOOL:[self importsGraphics] withName:@"importsGraphics"];
  [archiver encodeBOOL:[self usesFontPanel] withName:@"usesFontPanel"];
  [archiver encodeObject:[self font] withName:@"font"];
  [archiver encodeInt:[self alignment] withName:@"alignment"];
  [archiver encodeSize:[self maxSize] withName:@"maxSize"];
  [archiver encodeSize:[self minSize] withName:@"minSize"];
  [archiver encodeBOOL:[self isVerticallyResizable] withName:@"isVerticallyResizable"];
  [archiver encodeBOOL:[self isHorizontallyResizable] withName:@"isHorizontallyResizable"];
  [archiver encodeConditionalObject: [self delegate] withName: @"delegate"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver: unarchiver];

  [self setString: [unarchiver decodeStringWithName: @"string"]];
  [self setBackgroundColor:
	     [unarchiver decodeObjectWithName:@"backgroundColor"]];
  [self setDrawsBackground:
	     [unarchiver decodeBOOLWithName:@"drawsBackground"]];
  [self setTextColor:[unarchiver decodeObjectWithName:@"textColor"]];
  [self setEditable:[unarchiver decodeBOOLWithName:@"isEditable"]];
  [self setSelectable:[unarchiver decodeBOOLWithName:@"isSelectable"]];
  [self setFieldEditor:[unarchiver decodeBOOLWithName:@"isFieldEditor"]];
  [self setRichText:[unarchiver decodeBOOLWithName:@"isRichText"]];
  [self setImportsGraphics:[unarchiver decodeBOOLWithName:@"importsGraphics"]];
  [self setUsesFontPanel:[unarchiver decodeBOOLWithName:@"usesFontPanel"]];
  [self setFont:[unarchiver decodeObjectWithName:@"font"]];
  [self setAlignment:[unarchiver decodeIntWithName:@"alignment"]];
  [self setMaxSize:[unarchiver decodeSizeWithName:@"maxSize"]];
  [self setMinSize:[unarchiver decodeSizeWithName:@"minSize"]];
  [self setVerticallyResizable:
	    [unarchiver decodeBOOLWithName:@"isVerticallyResizable"]];
  [self setHorizontallyResizable:
	    [unarchiver decodeBOOLWithName:@"isHorizontallyResizable"]];
  [self setDelegate: [unarchiver decodeObjectWithName: @"delegate"]];

  return self;
}

@end /* NSText (GMArchiverMethods) */

@implementation NSTextView (GMArchiverMethods)

- (void)encodeWithModelArchiver:(GMArchiver*)archiver
{
  [super encodeWithModelArchiver:archiver];

  // Currently the text container is not encoded
  [archiver encodeSize:[self textContainerInset] withName:@"textContainerInset"];
  if ([self respondsToSelector: @selector(allowsUndo)])
    [archiver encodeBOOL:[self allowsUndo] withName:@"allowsUndo"];
  [archiver encodeBOOL:[self usesRuler] withName:@"usesRuler"];
  [archiver encodeBOOL:[self isRulerVisible] withName:@"isRulerVisible"];
  [archiver encodeObject:[self insertionPointColor] withName:@"insertionPointColor"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver: unarchiver];

  // Currently the text container is not encoded
  [self setTextContainerInset:[unarchiver decodeSizeWithName:@"textContainerInset"]];
  [self setAllowsUndo:[unarchiver decodeBOOLWithName:@"allowsUndo"]];
  [self setUsesRuler:[unarchiver decodeBOOLWithName:@"usesRuler"]];
  [self setRulerVisible:[unarchiver decodeBOOLWithName:@"isRulerVisible"]];
  [self setInsertionPointColor:
	    [unarchiver decodeObjectWithName:@"insertionPointColor"]];

  return self;
}

@end /* NSTextView (GMArchiverMethods) */
