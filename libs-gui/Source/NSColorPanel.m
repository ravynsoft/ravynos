/** <title>NSColorPanel</title>

   <abstract>System generic color panel</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   
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
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import "AppKit/NSBox.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSColorPanel.h"
#import "AppKit/NSColorPicker.h"
#import "AppKit/NSColorPicking.h"
#import "AppKit/NSColorWell.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSSlider.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSSplitView.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSDisplayServer.h"

#import "GSGuiPrivate.h"

#define MAX_ALPHA_VALUE 100.0
static NSLock *_gs_gui_color_panel_lock = nil;
static NSColorPanel *_gs_gui_color_panel = nil;
static int _gs_gui_color_picker_mask = NSColorPanelAllModesMask;
static int _gs_gui_color_picker_mode = NSWheelModeColorPanel;


@implementation NSApplication (NSColorPanel)

- (void) orderFrontColorPanel: sender
{ 
  NSColorPanel *colorPanel = [NSColorPanel sharedColorPanel];

  if (colorPanel)
    [colorPanel orderFront: nil];
  else
    NSBeep();
}

@end

@interface NSColorPanel (PrivateMethods)
- (void) _loadPickers;
- (void) _loadPickerAtPath: (NSString *)path;
- (void) _setupPickers;
- (void) _showNewPicker: (id)sender;
- (id) _initWithoutGModel;
- (void) _magnify: (id)sender;
@end

@implementation NSColorPanel (PrivateMethods)

- (void) _loadPickers
{
  NSArray	*paths;
  NSString	*path;
  NSEnumerator	*pathEnumerator;
  NSArray	*bundles;
  NSEnumerator	*bundleEnumerator;
  NSString	*bundleName;

  _pickers = [NSMutableArray new];

  paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory,
    NSAllDomainsMask, YES);

  pathEnumerator = [paths objectEnumerator];
  while ((path = [pathEnumerator nextObject]))
    {
      path = [path stringByAppendingPathComponent: @"ColorPickers"];
      bundles = [[NSFileManager defaultManager] directoryContentsAtPath: path];

      bundleEnumerator = [bundles objectEnumerator];
      while ((bundleName = [bundleEnumerator nextObject]))
	{
	  [self _loadPickerAtPath:
            [path stringByAppendingPathComponent: bundleName]];
	}
    }

  paths = [[NSBundle mainBundle] pathsForResourcesOfType: @"bundle"
                                             inDirectory: @"ColorPickers"];

  pathEnumerator = [paths objectEnumerator];
  while ((path = [pathEnumerator nextObject]))
    {
      [self _loadPickerAtPath: path];
    }
}

- (void) _loadPickerAtPath: (NSString *)path
{
  NSBundle	*bundle;
  Class		pickerClass;
  NSColorPicker	*picker;

  bundle = [NSBundle bundleWithPath: path];
  if (bundle && (pickerClass = [bundle principalClass]))
    {
      picker = [[pickerClass alloc] initWithPickerMask:_gs_gui_color_picker_mask
                                            colorPanel: self];
      if (picker && [picker conformsToProtocol:@protocol(NSColorPickingCustom)])
        {
          [(id<NSColorPickingCustom>)picker provideNewView: YES];
          [_pickers addObject: picker];
        }
      else
	{
	  NSLog(@"%@ does not contain a valid color picker.", path);
	}
    }
}

- (void) _setupPickers
{
  id<NSColorPickingDefault, NSColorPickingCustom> picker;
  NSButtonCell *cell;
  NSMutableArray *cells = [NSMutableArray new];
  int i, count;
  NSSize size = [_pickerMatrix frame].size;

  count = [_pickers count];
  for (i = 0; i < count; i++)
    {
      cell = [[_pickerMatrix prototype] copy];
      [cell setTag: i];
      picker = [_pickers objectAtIndex: i];
      [picker insertNewButtonImage: [picker provideNewButtonImage] in: cell];
      [cells addObject: cell];
    }

  [_pickerMatrix renewRows: 0 columns: count];
  [_pickerMatrix addRowWithCells: cells];
  RELEASE(cells);
  [_pickerMatrix setCellSize: NSMakeSize(size.width / count, size.height)];
  [_pickerMatrix setTarget: self];
  [_pickerMatrix setAction: @selector(_showNewPicker:)];

  // use the space occupied by the matrix of color picker buttons if the
  // button matrix is useless, i.e. it contains only one button
  if (count < 2)
    {
      [_pickerBox setFrame: NSUnionRect([_pickerBox frame],
                                        [_pickerMatrix frame])];
      [_pickerBox setNeedsDisplay: YES];
      // Display the only picker
      if (count == 1)
        [self _showNewPicker: _pickerMatrix];
    }
}

- (void) _showNewPicker: (id)sender
{
  _currentPicker = [_pickers objectAtIndex: [sender selectedColumn]];
  [_currentPicker setColor: [_colorWell color]];
  [_pickerBox setContentView: [_currentPicker provideNewView: NO]];
}

- (id) _initWithoutGModel
{
  NSRect contentRect = {{352, 519}, {200, 270}};
  NSSize maxContentSize = {500, 675};
  NSRect topViewRect = {{0, 0}, {200, 270}};
  NSRect magnifyRect = {{4, 230}, {50, 36}};
  NSRect wellRect = {{58, 230}, {138, 36}};
  NSRect matrixRect = {{4, 190}, {192, 36}};
  NSRect splitRect = {{0, 0}, {200, 190}};
  NSRect pickerViewRect = {{0, 40}, {200, 150}};
  NSRect pickerRect = {{0, 20}, {200, 130}};
  NSRect alphaRect = {{4, 4}, {160, 16}};
  NSRect swatchRect = {{4, 4}, {200, 30}};
  NSView *v;
  NSButtonCell *pickerButton;
  NSView *pickerView;
  NSView *swatchView;
  NSColorWell *well;
  int i;
  unsigned int style = NSTitledWindowMask | NSClosableWindowMask
                      | NSResizableWindowMask | NSUtilityWindowMask;

  self = [super initWithContentRect: contentRect 
			  styleMask: style
			    backing: NSBackingStoreRetained
			      defer: NO
			     screen: nil];
  if (nil == self)
    return nil;

  [self setTitle: _(@"Colors")];
  [self setBecomesKeyOnlyIfNeeded: YES];
  [self setContentMinSize: contentRect.size];
  [self setContentMaxSize: maxContentSize];
  v = [self contentView];

  _topView = [[NSView alloc] initWithFrame: topViewRect];
  [_topView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [v addSubview: _topView];
  RELEASE(_topView);

  _magnifyButton = [[NSButton alloc] initWithFrame: magnifyRect];
  [_magnifyButton setAutoresizingMask: (NSViewMaxXMargin | NSViewMinYMargin)];
  [_magnifyButton setImage: [NSImage imageNamed: @"MagnifyGlass"]];
  [_magnifyButton setBordered: YES];
  [_magnifyButton setAction: @selector(_magnify:)];
  [_magnifyButton setTarget: self];
  [_topView addSubview: _magnifyButton];

  _colorWell = [[NSColorWell alloc] initWithFrame: wellRect];
  [_colorWell setAutoresizingMask: (NSViewWidthSizable | NSViewMinYMargin)];
  [_colorWell setBordered: NO];
  [_colorWell setTarget: self];
  [_colorWell setAction: @selector(_updatePicker:)];
  [_topView addSubview: _colorWell];

  // Prototype cell for the matrix
  pickerButton = [[NSButtonCell alloc] initImageCell: nil];
  [pickerButton setButtonType: NSOnOffButton];
  [pickerButton setBordered: YES];

  _pickerMatrix = [[NSMatrix alloc] initWithFrame: matrixRect
				    mode: NSRadioModeMatrix
				    prototype: pickerButton
				    numberOfRows: 0
				    numberOfColumns: 0];
  RELEASE(pickerButton);
  [_pickerMatrix setAutoresizingMask: (NSViewWidthSizable | NSViewMinYMargin)];
  [_pickerMatrix setCellSize: matrixRect.size];
  [_pickerMatrix setIntercellSpacing: NSMakeSize(1, 0)];
  [_pickerMatrix setAutosizesCells: YES];
  [_topView addSubview: _pickerMatrix];

  _splitView = [[NSSplitView alloc] initWithFrame: splitRect];
  [_splitView setVertical: NO];
  [_splitView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [_topView addSubview: _splitView];

  pickerView = [[NSView alloc] initWithFrame: pickerViewRect];
  [pickerView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];

  _pickerBox = [[NSBox alloc] initWithFrame: pickerRect];
  [_pickerBox setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  [_pickerBox setBorderType: NSNoBorder];
  [_pickerBox setTitle: @""];
  [_pickerBox setTitlePosition: NSNoTitle];
  [pickerView addSubview: _pickerBox];

  _alphaSlider = [[NSSlider alloc] initWithFrame: alphaRect];
  [_alphaSlider setAutoresizingMask: (NSViewWidthSizable | NSViewMaxYMargin)];
  [_alphaSlider setMinValue: 0.0];
  [_alphaSlider setMaxValue: MAX_ALPHA_VALUE];
  [_alphaSlider setFloatValue: MAX_ALPHA_VALUE];
  [_alphaSlider setContinuous: YES];
  [_alphaSlider setTitle: _(@"Opacity")];
  [[_alphaSlider cell] setBezeled: YES];
  [_alphaSlider setTarget: self];
  [_alphaSlider setAction: @selector(_alphaChanged:)];
  [pickerView addSubview: _alphaSlider];
  _showsAlpha = YES;

  swatchView = [[NSView alloc] initWithFrame: swatchRect];
  [swatchView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
  // Add all the subviews at the end
  [_splitView addSubview: pickerView];
  [_splitView addSubview: swatchView];
  RELEASE(pickerView);
  RELEASE(swatchView);

  // FIXME: This should be loaded form somewhere. 
  // Perhaps a colour list called "custom"? 
  for (i = 0; i < 14; i++)
  { 
    NSColor *colour;
    
    switch (i)
      {
	default:
	case 0: colour = [NSColor greenColor]; break;
	case 1: colour = [NSColor whiteColor]; break;
	case 2: colour = [NSColor blackColor]; break;
	case 3: colour = [NSColor blueColor]; break;
	case 4: colour = [NSColor brownColor]; break;
	case 5: colour = [NSColor cyanColor]; break;
	case 6: colour = [NSColor darkGrayColor]; break;
	case 7: colour = [NSColor grayColor]; break;
	case 8: colour = [NSColor lightGrayColor]; break;
	case 9: colour = [NSColor magentaColor]; break;
	case 10: colour = [NSColor orangeColor]; break;
	case 11: colour = [NSColor purpleColor]; break;
	case 12: colour = [NSColor redColor]; break;
	case 13: colour = [NSColor yellowColor]; break;
      }
    well = [[NSColorWell alloc] initWithFrame: NSMakeRect(i * 13 + 5, 5, 12, 12)];
    [well setColor: colour];
    [well setBordered: NO];
    [well setEnabled: YES];
    [well setTarget: self];
    [well setAction: @selector(_bottomWellAction:)];
    [swatchView addSubview: well];
    RELEASE(well);
  }

  return self;
}

- (void) _alphaChanged: (id) sender
{
  [self setColor: [[self color] colorWithAlphaComponent: [self alpha]]];
}

- (void) _apply: (id) sender
{
  // This is currently not used
}

- (void) _magnify: (id) sender
{
  NSEvent *currentEvent;
  NSCursor *cursor;

  [self _captureMouse: self];

  /**
   * There was code here to dynamically generate a magnifying glass
   * cursor with a magnified portion of the screenshot in it,
   * but changing the cursor rapidly on X seems to cause flicker,
   * so we just use a plain magnifying glass. (dynamic code is in r33543)
   */
  cursor = [[[NSCursor alloc] initWithImage: [NSImage imageNamed: @"MagnifyGlass"]
					      hotSpot: NSMakePoint(12, 13)] autorelease];
  [cursor push];

  NS_DURING
    {
      do {
	NSPoint mouseLoc;
	NSImage *img;
        CREATE_AUTORELEASE_POOL(pool);
 	
	currentEvent = [NSApp nextEventMatchingMask: NSLeftMouseDownMask | NSLeftMouseUpMask | NSMouseMovedMask
					  untilDate: [NSDate distantFuture]
					     inMode: NSEventTrackingRunLoopMode
					    dequeue: YES];
	
	mouseLoc = [self convertBaseToScreen: [self mouseLocationOutsideOfEventStream]];
	
	img = [GSCurrentServer() contentsOfScreen: [[self screen] screenNumber]
					   inRect: NSMakeRect(mouseLoc.x, mouseLoc.y, 1, 1)];
	
	if (img != nil)
	  {
	    NSBitmapImageRep *rep = (NSBitmapImageRep *)[img bestRepresentationForDevice: nil];
	    NSColor *color = [rep colorAtX: 0 y: 0];
	    [self setColor: color];
	  }
       [pool drain];
     } while ([currentEvent type] != NSLeftMouseUp && 
	       [currentEvent type] != NSLeftMouseDown);
    }
  NS_HANDLER
    {
      NSLog(@"Exception occurred in -[NSColorPanel _magnify:]: %@",
	    localException);
    }
  NS_ENDHANDLER
    
  [NSCursor pop];
  [self _releaseMouse: self];
}

- (void) _updatePicker: (id) sender
{
  [self setColor: [_colorWell color]];
}

- (void) _bottomWellAction: (id) sender
{
  [self setColor: [sender color]];
}

@end

/**<p>TODO Description</p>
 */
@implementation NSColorPanel

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSColorPanel class])
    {
      // Initial version
      [self setVersion: 1];
      _gs_gui_color_panel_lock = [NSLock new];
    }
}

/**<p>Creates ( if needed ) and returns the shared NSColorPanel.</p>
 */
+ (NSColorPanel *)sharedColorPanel
{
  if (_gs_gui_color_panel == nil)
    {
      [_gs_gui_color_panel_lock lock];
      if (!_gs_gui_color_panel)
        {
          //  if (![NSBundle loadNibNamed: @"ColorPanel" owner: self]);

	  // Keep this two lines separated so the check in [init] works.
	  _gs_gui_color_panel = [self alloc];
	  [_gs_gui_color_panel init];
        }
      [_gs_gui_color_panel_lock unlock];
    }

  return _gs_gui_color_panel;
}

/** <p>Returns whether the NSColorPanel has been already created.</p>
 */
+ (BOOL) sharedColorPanelExists
{
  return (_gs_gui_color_panel == nil) ? NO : YES;
}

+ (void) setPickerMask: (int)mask
{
  _gs_gui_color_picker_mask = mask;
}

/**
 */
+ (void) setPickerMode: (int)mode
{
  _gs_gui_color_picker_mode = mode;
}

/**<p>Drags <var>aColor</var> from <var>sourceView</var> at the location
   give by the event <var>anEvent</var> ( [NSView-convertPoint:fromView:] ).
   The type declared into the pasteboard is NSColorPboardType</p>
   <p>See Also: [NSView-convertPoint:fromView:] 
   [NSView-dragImage:at:offset:event:pasteboard:source:slideBack:</p>
 */
+ (BOOL) dragColor: (NSColor *)aColor
	 withEvent: (NSEvent *)anEvent
	  fromView: (NSView *)sourceView
{
  NSPasteboard	*pb = [NSPasteboard pasteboardWithName: NSDragPboard];
  NSImage	*image = [NSImage imageNamed: @"common_ColorSwatch"];
  NSSize	size;
  NSPoint	point;

  [pb declareTypes: [NSArray arrayWithObjects: NSColorPboardType, nil]
      owner: aColor];
  [aColor writeToPasteboard: pb];
  [image setBackgroundColor: aColor];

  size = [image size];
  point = [sourceView convertPoint: [anEvent locationInWindow] fromView: nil];
  point.x -= size.width/2;
  point.y -= size.width/2;

  [sourceView dragImage: image
                     at: point
                 offset: NSMakeSize(0,0)
                  event: anEvent
             pasteboard: pb
                 source: sourceView
              slideBack: NO];

  return YES;
}

/*
 * Instance methods
 */

- (id) init
{
  if (self != _gs_gui_color_panel)
  {
      RELEASE(self);
      return _gs_gui_color_panel;
  }

  [self _initWithoutGModel];

  [self _loadPickers];
  [self _setupPickers];
  [self setMode: _gs_gui_color_picker_mode];
  [self setShowsAlpha: ![NSColor ignoresAlpha]];

  return self;
}

- (void) dealloc
{
  // As there is only one this will never be called

  RELEASE(_topView);
  RELEASE(_colorWell);
  RELEASE(_magnifyButton);
  RELEASE(_pickerMatrix);
  RELEASE(_pickerBox);
  RELEASE(_alphaSlider);
  RELEASE(_splitView);
  RELEASE(_pickers);
  [super dealloc];
}

/** <p>Returns the NSColorPanel's accessory view if it exists, 
    nil otherwise.</p><p>See Also: -setAccessoryView:</p>    
 */
- (NSView *) accessoryView
{
  return _accessoryView;
}

/** <p> Returns whether the NSColorPanel continuously sends its action message.
    </p><p>See Also: -setContinuous:-setAction: -setTarget: </p>
 */
- (BOOL) isContinuous
{
  return _isContinuous;
}

/** <p> Returns the current mode of the NSColorPanel.</p>
    <p>See Also: -setMode:</p>
 */
- (int) mode
{
  if (_currentPicker != nil)
    return [_currentPicker currentMode];
  else
    return 0;
}

/**<p> Sets the accessoryView to a view. The old view ( if exists ) will be 
    removed ( and released ). You need to retain it if you want to use
    it later</p>
    <p>See Also: -accessoryView</p>
 */
- (void) setAccessoryView: (NSView *)aView
{
  if (_accessoryView == aView)
    return;

  if (_accessoryView != nil)
    [_accessoryView removeFromSuperview];
  _accessoryView = aView;
  [_splitView addSubview: _accessoryView];
}

/**<p>Sets the NSColorPanl action method to <var>aSelector</var> The
   action message is usally send in -setColor:, when the picker is updated,
   when a new picker is shown, when the alpha is changed or when one of the
   color wells at the bottom is selected.</p>
 */
- (void) setAction: (SEL)aSelector
{
  _action = aSelector;
}

/** <p>Sets whether the NSColorPanel sends continuously action messages</p>
    <p>See Also: -isContinuous</p>
 */
- (void) setContinuous: (BOOL)flag
{
  _isContinuous = flag;
}

/** <p>Set the NSColorPanel mode to <var>mode</var>.</p>
    <p>See Also: -mode</p>
 */
- (void) setMode: (int)mode
{
  int i, count;

  if (mode == [self mode])
    return;

  count = [_pickers count];
  for (i = 0; i < count; i++)
    {
      if ([[_pickers objectAtIndex: i] supportsMode: mode])
        {
	  [_pickerMatrix selectCellWithTag: i];
	  [self _showNewPicker: _pickerMatrix];
	  [_currentPicker setMode: mode];
	  break;
	}
    }
}

/** <p>Sets whether the NSColorPanel shows alpha values and the alpha
    slider.</p><p>See Also: -showsAlpha</p>
 */
- (void) setShowsAlpha: (BOOL)flag
{
  if (flag == _showsAlpha)
    return;

  if (flag)
    {
      NSRect newFrame = [_pickerBox frame];
      CGFloat offset = [_alphaSlider frame].size.height + 4.0;

      [_alphaSlider setFrameOrigin: newFrame.origin];
      [[_pickerBox superview] addSubview: _alphaSlider];
      newFrame.origin.y += offset;
      newFrame.size.height -= offset;
      [_pickerBox setFrame: newFrame];
    }
  else
    {
      // Remove the alpha slider, and add its size to the pickerBox
      [_alphaSlider removeFromSuperview];
      [_pickerBox setFrame: NSUnionRect([_pickerBox frame],
                                        [_alphaSlider frame])];
    }

  _showsAlpha = flag;

  [_pickers makeObjectsPerformSelector: @selector(alphaControlAddedOrRemoved:)
                            withObject: self];

  [_topView setNeedsDisplay: YES];
}

/** <p>Sets the target object to <var>anObject</var></p>
 */
- (void) setTarget: (id)anObject
{
  _target = anObject;
}

/** <p>Returns whether the NSColorPanel shows alpha values and the alpha
    slider</p><p>See Also: -setShowsAlpha:</p>
 */
- (BOOL) showsAlpha
{
  return _showsAlpha;
}

//
// Attaching a Color List
//
- (void) attachColorList: (NSColorList *)aColorList
{
  [_pickers makeObjectsPerformSelector: @selector(attachColorList:)
                            withObject: aColorList];
}

- (void) detachColorList: (NSColorList *)aColorList
{
  [_pickers makeObjectsPerformSelector: @selector(detachColorList:)
                            withObject: aColorList];
}

/** <p>Returns the alpha value of the NSColorPanel. Returns
    1.0 if the NSColorPanel does not show alpha</p>
    <p>See Also: -showsAlpha -setShowsAlpha:</p>    
 */
- (CGFloat) alpha
{
  if ([self showsAlpha])
    return [_alphaSlider floatValue] / MAX_ALPHA_VALUE;
  else
    return 1.0;
}

/** <p>Returns the current NSColor displayed by the NSColorPanel.</p>
    <p>See Also : -setColor:</p>
 */
- (NSColor *) color
{
  return [_colorWell color];
}

/** <p>Sets the NSColor displayed to aColor. This method posts a
    NSColorPanelColorDidChangeNotification notification if needed.</p>
    <p>See Also: -color [NSColorWell-setColor:]
    </p>
*/
- (void) setColor: (NSColor *)aColor
{
  [_colorWell setColor: aColor];
  [_currentPicker setColor: aColor];
  if ([self showsAlpha])
    [_alphaSlider setFloatValue: [aColor alphaComponent] * MAX_ALPHA_VALUE];

  // FIXME: to support [self isContinuous] the API between color pickers
  // and NSColorPanel needs some changes. Currently the color panel is
  // always continuous

  [NSApp sendAction: @selector(changeColor:) to: nil from: self];
  if ((_action) && (_target != nil))
    [NSApp sendAction: _action to: _target from: self];  

  [[NSNotificationCenter defaultCenter]
      postNotificationName: NSColorPanelColorDidChangeNotification
                    object: (id)self];
}

- (BOOL) worksWhenModal
{
  return YES;
}

@end
