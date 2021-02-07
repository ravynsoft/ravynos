/** <title>NSAlert</title>

   <abstract>Encapsulate an alert panel</abstract>

   Copyright <copy>(C) 1998, 2000, 2004 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: July 2004

   GSAlertPanel and alert panel functions implementation
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1998

   GSAlertPanel and alert panel functions cleanup and improvements (scroll view)
   Author: Pascal J. Bourguignon <pjb@imaginet.fr>>
   Date: 2000-03-08

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

#import <Foundation/NSDebug.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSError.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>
#import "AppKit/NSAlert.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSHelpManager.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextView.h"

#import "GNUstepGUI/GMAppKit.h"
#import "GNUstepGUI/GMArchiver.h"
#import "GSGuiPrivate.h"

extern NSThread *GSAppKitThread;

static NSNotificationCenter *nc = nil;

#ifdef ALERT_TITLE
static NSString	*defaultTitle = @"Alert";
#else
static NSString	*defaultTitle = @" ";
#endif

/*

 +--------------------------------------------------------------------------+
 |############################Title bar#####################################|
 +--------------------------------------------------------------------------+
 |       |           |   |    |                                             |
 |  ...........      |   |    |                                             |
 |  :       | :      |   |    |                                             |
 |--:  Icon | :----Title |    |                                             |
 |  :-------|-:          |    |                                             |
 |  :.........:          |    |                                             |
 |                       |    |                                             |
 |-===========================|=============~~~~~~=========================-|
 |                       s    |                                             |
 |                       s    |                                             |
 |    ...................s....|.........................................    |
 |    : Message           s                    s                        ;    |
 |    :                  s                    s                        :    |
 |    :                  s                    s                        :    |
 |----:                  s                    s                        :----|
 |    :~~~~~~~~~~~~~~~~~~s~~~~~~~~~~~~~~~~~~~~s~~~~~~~~~~~~~~~~~~~~~~~~:    |
 |    :                  s                    s                        :    |
 |    :..................s.............................................:    |
 |             |         s                                                  |
 |             |         s +-----------+   +-----------+   +-----------+    |
 |             |         s |  Altern   |---|  Cancel   |---|    OK     |----|
 |             |         s +-----------+   +-----------+   +-----------+    |
 |             |         s      |                 |             |           |
 +--------------------------------------------------------------------------+

    Apart from  the buttons and window  borders, '|' and  '-' mean not
    flexible, while '~' and 's' mean flexible.

    The global  window size  is determined by  the message  text field
    size.  which  is computed with  sizeToFit. However, if  the window
    would become larger  than the screen, then the  message text field
    is replaced with a scroll view containing the text.


    The strategy taken  in this new version of  GSAlertPanel is to let
    the icon,  the title  field and the  line (box view)  being placed
    automatically  by  resizing  the   window,  but  to  always  place
    explicitely the  message field (or  scroll view when  it's needed)
    and  the buttons  (that may  change of  width), the  whole  in the
    sizeToFitIfNeeded  method.  We're  doing  it separately  from  the
    setting  of the elements  (setTitle: ...) because  we also  need to
    recompute the size of the window and position of the elements when
    dearchiving a panel, because  it could be dearchived and displayed
    on a much smaller screen  than originaly created, in which case we
    needed to embed  the message field into a  scroll view, and reduce
    the size of the window.


    Some rules (which are implemented in sizePanelToFit):
    =====================================================


    IF   the messageField is too big either vertically or horizontally and
         would make the window greater than the screen in any direction,
    THEN use a scroll view.


    The width of the window content rect is the minimum of:
      = the width of the content rect of a window filling the screen
      = the maximum of:
        = a minimum width of 362,
        = the messageField width + 2 = MessageHorzMargin,
        = the sum of sizes of the buttons and their interspaces and margins,
        = the sum of sizes of the icon, title and their interspaces
          and margin.


    The height of the window content rect is the minimum of:
      = the height of the content rect of a window filling the screen
      = the maximum of:
        = a minimum height of 161.
	= the sum of:
          = The height of the icon, the line and their interspaces.
          = The height of the messageField and its interspaces, if present.
          = The height of the buttons and their interspaces, if present.


    The size of the scroll view must be at minimum ScrollMinSize
    in each direction.

    The size of the messageField is a given (sizeToFit).

    The height of the scroll is the rest of the height of the content rect
    minus the height of the other elements.

    The width of the scroll is the rest of the width of the content rect
    minus the margins.

    In order to prevent alert panels from obscuring the whole screen, we
    limit the width and height of a panel to at most a factor of SIZE_SCALE
    of the screen size. At present, we use 60% for SIZE_SCALE, which means
    that the limit is greater than the minimum width and height even on a
    640x400 screen.

    ((wsize.width <= ssize.width * SIZE_SCALE)
     and ([messageField frame].size.width+2*MessageHorzMargin <= wsize.width))
    or ((wsize.width == ssize.width * SIZE_SCALE)
        and ([scroll frame].size.width = wsize.width-2*MessageHorzMargin));

    ...
*/


@class	GSAlertPanel;

static GSAlertPanel	*standardAlertPanel = nil;
static GSAlertPanel	*informationalAlertPanel = nil;
static GSAlertPanel	*criticalAlertPanel = nil;

@interface	GSAlertPanel: NSPanel
{
  NSButton	*defButton;
  NSButton	*altButton;
  NSButton	*othButton;
  NSButton	*icoButton;
  NSTextField	*titleField;
  NSTextField	*messageField;
  NSScrollView	*scroll;
  NSInteger	result;
  BOOL          isGreen;  // we were unarchived and not resized.
}

- (id) _initWithoutGModel;
- (NSInteger) runModal;
- (void) setTitleBar: (NSString*)titleBar
		icon: (NSImage*)icon
	       title: (NSString*)title
	     message: (NSString*)message;
- (void) setTitleBar: (NSString*)titleBar
		icon: (NSImage*)icon
	       title: (NSString*)title
	     message: (NSString*)message
		 def: (NSString*)defaultButton
		 alt: (NSString*)alternateButton
	       other: (NSString*)otherButton;
- (void) setButtons: (NSArray *)buttons;
- (void) sizePanelToFit;
- (void) buttonAction: (id)sender;
- (NSInteger) result;
- (BOOL) isActivePanel;
@end


@implementation	GSAlertPanel

//static const float WTitleHeight = 0.0;      // TODO: Check this value.
static const float WinMinWidth = 362.0;
static const float WinMinHeight = 161.0;
static const float IconSide = 48.0;
static const float IconBottom = -56.0;       // from the top of the window.
static const float IconLeft = 8.0;
static const float TitleLeft = 64.0;
static const float TitleMinRight = 8.0;
static const float LineHeight = 2.0;
static const float LineBottom = -66.0;       // from the top of the window.
static const float LineLeft = 0.0;
//static const float ScrollMinSize = 48.0;     // in either direction.
static const float MessageHorzMargin = 8.0;  // 5 is too little margin.
//static const float MessageMinHeight = 20.0;
static const float MessageVertMargin = 6.0;  // from the top of the buttons.
static const float MessageTop = -72;         // from the top of the window;
static const float ButtonBottom = 8.0;       // from the bottom of the window.
static const float ButtonMargin = 8.0;
static const float ButtonInterspace = 10.0;
static const float ButtonMinHeight = 24.0;
static const float ButtonMinWidth = 72.0;

#define SIZE_SCALE 0.6
#define MessageFont [NSFont messageFontOfSize: 14]

+ (void) initialize
{
  if (self == [GSAlertPanel class])
    {
      [self setVersion: 1];
    }
}

- (id) init
{
/*
  if (![NSBundle loadNibNamed: @"AlertPanel" owner: self])
    {
      NSLog(@"cannot open alert panel model file\n");
      return nil;
    }
 */
  return [self _initWithoutGModel];
}

- (void) dealloc
{
  if (self == standardAlertPanel)
    {
      standardAlertPanel = nil;
    }
  if (self == informationalAlertPanel)
    {
      informationalAlertPanel = nil;
    }
  if (self == criticalAlertPanel)
    {
      criticalAlertPanel = nil;
    }
  RELEASE(defButton);
  RELEASE(altButton);
  RELEASE(othButton);
  RELEASE(icoButton);
  RELEASE(titleField);
  RELEASE(messageField);
  RELEASE(scroll);
  [super dealloc];
}

static NSScrollView*
makeScrollViewWithRect(NSRect rect)
{
  float		lineHeight = [MessageFont boundingRectForFont].size.height;
  NSScrollView	*scroll = [[NSScrollView alloc]initWithFrame: rect];

  [scroll setBorderType: NSLineBorder];
  [scroll setBackgroundColor: [NSColor controlBackgroundColor]];
  [scroll setHasHorizontalScroller: NO];
  [scroll setHasVerticalScroller: YES];
  [scroll setScrollsDynamically: YES];
  [scroll setLineScroll: lineHeight];
  [scroll setPageScroll: lineHeight*10.0];
  return scroll;
}

- (NSButton*) _makeButtonWithRect: (NSRect)rect tag: (NSInteger)tag
{
  NSButton *button = [[NSButton alloc] initWithFrame: rect];

  [button setAutoresizingMask: NSViewMinXMargin | NSViewMaxYMargin];
  [button setButtonType: NSMomentaryPushInButton];
  [button setTitle: @""];
  [button setTarget: self];
  [button setAction: @selector(buttonAction:)];
  [button setTag: tag];
  [button setFont: [NSFont systemFontOfSize: 0]];
  return button;
}

#define useControl(control)  ([control superview] != nil)

static void
setControl(NSView* content, id control, NSString *title)
{
  if (title != nil)
    {
      if ([control respondsToSelector: @selector(setTitle:)])
	{
	  [control setTitle: title];
	}
      else if ([control respondsToSelector: @selector(setStringValue:)])
	{
	  [control setStringValue: title];
	}
      [control sizeToFit];
      if (!useControl(control))
	{
	  [content addSubview: control];
	}
    }
  else if (useControl(control))
    {
      [control removeFromSuperview];
    }
}

static void
setButton(NSView* content, NSButton *control, NSButton *template)
{
  if (template != nil)
    {
      [control setTitle: [template title]];
      [control setKeyEquivalent: [template keyEquivalent]];
      [control setKeyEquivalentModifierMask:
		 [template keyEquivalentModifierMask]];
      [control setTag: [template tag]];
      [control sizeToFit];
      if (!useControl(control))
	{
	  [content addSubview: control];
	}
    }
  else if (useControl(control))
    {
      [control removeFromSuperview];
    }
}

static void
setKeyEquivalent(NSButton *button)
{
  NSString *title = [button title];
  
  if (![[button keyEquivalent] isEqualToString: @"\r"])
    {
      if ([title isEqualToString: _(@"Cancel")])
	{
	  [button setKeyEquivalent: @"\e"];
	  [button setKeyEquivalentModifierMask: 0];
	}
      else if ([title isEqualToString: _(@"Don't Save")])
	{
	  [button setKeyEquivalent: @"d"];
	  [button setKeyEquivalentModifierMask: NSCommandKeyMask];
	}
      else
	{
	  [button setKeyEquivalent: @""];
	  [button setKeyEquivalentModifierMask: 0];
	}
    }
}

- (id) _initWithoutGModel
{
  NSRect  rect;
  NSImage *image;
  NSBox	  *box;
  NSView  *content;
  NSRect  r = NSMakeRect(0.0, 0.0, WinMinWidth, WinMinHeight);
  NSFont  *titleFont = [NSFont systemFontOfSize: 18.0];
  float   titleHeight = [titleFont boundingRectForFont].size.height;
  NSText  *fieldEditor = [self fieldEditor: YES forObject: messageField];
  NSDictionary *selectedAttrs;


  self = [self initWithContentRect: r
	       styleMask: NSTitledWindowMask
	       backing: NSBackingStoreRetained
	       defer: YES];

  if (self == nil)
    return nil;

  [self setTitle: @" "];
  [self setLevel: NSModalPanelWindowLevel];
  content = [self contentView];
  
  // we're an ATTENTION panel, therefore:
  [self setHidesOnDeactivate: NO];
  [self setBecomesKeyOnlyIfNeeded: NO];
  
  // First, the subviews that will be positioned automatically.
  rect.size.height = IconSide;
  rect.size.width = IconSide;
  rect.origin.y = r.origin.y + r.size.height + IconBottom;
  rect.origin.x = IconLeft;
  icoButton = [[NSButton alloc] initWithFrame: rect];
  [icoButton setAutoresizingMask: NSViewMaxXMargin|NSViewMinYMargin];
  [icoButton setBordered: NO];
  [icoButton setEnabled: NO];
  [[icoButton cell] setImageDimsWhenDisabled: NO];
  [[icoButton cell] setImageScaling: NSImageScaleProportionallyUpOrDown];
  [icoButton setImagePosition: NSImageOnly];
  image = [[NSApplication sharedApplication] applicationIconImage];
  [icoButton setImage: image];
  [content addSubview: icoButton];

  // Title
  rect.size.height = 0.0; // will be sized to fit anyway.
  rect.size.width = 0.0;  // will be sized to fit anyway.
  rect.origin.y = r.origin.y + r.size.height 
                  + IconBottom + (IconSide - titleHeight)/2;;
  rect.origin.x = TitleLeft;
  titleField = [[NSTextField alloc] initWithFrame: rect];
  [titleField setAutoresizingMask: NSViewMinYMargin];
  [titleField setEditable: NO];
  [titleField setSelectable: NO];
  [titleField setBezeled: NO];
  [titleField setDrawsBackground: NO];
  [titleField setStringValue: @""];
  [titleField setFont: titleFont];

  // Horizontal line
  rect.size.height = LineHeight;
  rect.size.width = r.size.width;
  rect.origin.y = r.origin.y + r.size.height + LineBottom;
  rect.origin.x = LineLeft;
  box = [[NSBox alloc] initWithFrame: rect];
  [box setAutoresizingMask: NSViewWidthSizable | NSViewMinYMargin];
  [box setTitlePosition: NSNoTitle];
  [box setBorderType: NSGrooveBorder];
  [content addSubview: box];
  RELEASE(box);
  
  // Then, make the subviews that'll be sized by sizePanelToFit;
  rect.size.height = 0.0;
  rect.size.width = 0.0;
  rect.origin.y = 0.0;
  rect.origin.x = 0.0;
 
  messageField = [[NSTextField alloc] initWithFrame: rect];
  [messageField setEditable: NO];
  /*
    PJB:
    How do you  want the user to report an error  message if it is
    not selectable?  Any text visible on the  screen should always
    be selectable for a copy-and-paste. Hence, setSelectable: YES.
  */
  selectedAttrs = [NSDictionary dictionaryWithObjectsAndKeys:
				 [NSColor controlLightHighlightColor], 
				 NSBackgroundColorAttributeName,
				 nil];
  [(NSTextView *)fieldEditor setSelectedTextAttributes: selectedAttrs]; 
  [messageField setSelectable: YES];
  [messageField setBezeled: NO];
  [messageField setDrawsBackground: NO];
  [messageField setAlignment: NSCenterTextAlignment];
  [messageField setStringValue: @""];
  [messageField setFont: MessageFont];
  
  defButton = [self _makeButtonWithRect: rect tag: NSAlertDefaultReturn];
  [defButton setKeyEquivalent: @"\r"];
  [defButton setHighlightsBy: NSPushInCellMask | NSChangeGrayCellMask 
                              | NSContentsCellMask];
  [defButton setImagePosition: NSImageRight];
  [defButton setImage: [NSImage imageNamed: @"common_ret"]];
  [defButton setAlternateImage: [NSImage imageNamed: @"common_retH"]];
  
  altButton = [self _makeButtonWithRect: rect tag: NSAlertAlternateReturn];
  othButton = [self _makeButtonWithRect: rect tag: NSAlertOtherReturn];
  
  rect.size.height = 80.0;
  scroll = makeScrollViewWithRect(rect);
  
  result = NSAlertErrorReturn;
  isGreen = YES;

  return self;
}

- (void) sizePanelToFit
{
  NSRect	bounds;
  NSSize	ssize; 			// screen size (corrected).
  NSSize	bsize; 			// button size (max of the three).
  NSSize	wsize = {0.0, 0.0}; 	// window size (computed).
  NSScreen	*screen;
  NSView	*content;
  NSButton	*buttons[3];
  float		position = 0.0;
  int		numberOfButtons;
  int		i;
  BOOL		needsScroll;
  BOOL		couldNeedScroll;
  NSUInteger	mask = [self styleMask];

  /*
   * Set size to the size of a content rectangle of a panel
   * that completely fills the screen.
   */
  screen = [self screen];
  if (screen == nil)
    {
      screen = [NSScreen mainScreen];
    }
  bounds = [screen frame];
  bounds = [NSWindow contentRectForFrameRect: bounds styleMask: mask];
  ssize = bounds.size;
  /* Do not let a panel grow beyond a factor of SIZE_SCALE of the screen */
  ssize.width = SIZE_SCALE * ssize.width;
  ssize.height = SIZE_SCALE * ssize.height;

  // Let's size the title.
  if (useControl(titleField))
    {
      NSRect	rect = [titleField frame];
      float	width = TitleLeft + rect.size.width + TitleMinRight;

      if (wsize.width < width)
	{
	  wsize.width = width;
	  // ssize.width < width = > the title will be silently clipped.
	}
    }

  wsize.height = -LineBottom;


  // Let's count the buttons.
  bsize.width = ButtonMinWidth;
  bsize.height = ButtonMinHeight;
  buttons[0] = defButton;
  buttons[1] = altButton;
  buttons[2] = othButton;
  numberOfButtons = 0;
  for (i = 0; i < 3; i++)
    {
      if (useControl(buttons[i]))
	{
	  NSRect rect = [buttons[i] frame];

	  if (bsize.width < rect.size.width)
	    {
	      bsize.width = rect.size.width;
	    }
	  if (bsize.height < rect.size.height)
	    {
	      bsize.height = rect.size.height;
	    }
	  numberOfButtons++;
	}
    }

  if (numberOfButtons > 0)
    {
      // (with NSGetAlertPanel, there could be zero buttons).
      float	width = (bsize.width + ButtonInterspace) * numberOfButtons
		  -ButtonInterspace + ButtonMargin * 2;
      /*
       * If the buttons are too wide or too high to fit in the screen,
       * then too bad! Thought it would be simple enough to put them
       * in the scroll view with the messageField.
       * TODO: See if we raise an exception here or if we let the
       *       QA people detect this kind of problem.
       */
      if (wsize.width < width)
	{
	  wsize.width = width;
	}
      wsize.height += ButtonBottom + bsize.height;
    }

  // Let's see the size of the messageField and how to place it.
  needsScroll = NO;
  couldNeedScroll = useControl(messageField);
  if (couldNeedScroll)
    {
      NSRect	rect = [messageField frame];
      float	width = rect.size.width + 2*MessageHorzMargin;

      if (wsize.width < width)
	{
	  wsize.width = width;
	}

      /* If the message is too wide, wrap its text. Apparently, we cannot
	 use -sizeToFit to compute the height of the message. */
      [messageField setAlignment: NSLeftTextAlignment];
      width = ssize.width - 2*MessageHorzMargin;
      rect.size =
	[[messageField attributedStringValue]
	  boundingRectWithSize: NSMakeSize(width, 1e6)
		       options: 0].size;
      [messageField setFrame: rect];

      /*
       * But only the messageField can impose a great height, therefore
       * we check it along in the next paragraph.
       */
      wsize.height += rect.size.height + 2 * MessageVertMargin;
    }
  else
    {
      wsize.height += MessageVertMargin;
    }

  // Strategically placed here, we resize the window.
  if (ssize.height < wsize.height)
    {
      wsize.height = ssize.height;
      needsScroll = couldNeedScroll;
    }
  else if (wsize.height < WinMinHeight)
    {
      wsize.height = WinMinHeight;
    }
  if (needsScroll)
    {
      wsize.width += [NSScroller scrollerWidth] + 4.0;
    }
  if (ssize.width < wsize.width)
    {
      wsize.width = ssize.width;
    }
  else if (wsize.width < WinMinWidth)
    {
      wsize.width = WinMinWidth;
    }
  bounds = NSMakeRect(0, 0, wsize.width, wsize.height);
  bounds = [NSWindow frameRectForContentRect: bounds styleMask: mask];
  [self setMaxSize: bounds.size];
  [self setMinSize: bounds.size];
  [self setContentSize: wsize];
  content = [self contentView];
  bounds = [content bounds];

  // Now we can place the buttons.
  if (numberOfButtons > 0)
    {
      position = bounds.origin.x + bounds.size.width - ButtonMargin;
      for (i = 0; i < 3; i++)
	{
	  if (useControl(buttons[i]))
	    {
	      NSRect	rect;

	      position -= bsize.width;
	      rect.origin.x = position;
	      rect.origin.y = bounds.origin.y + ButtonBottom;
	      rect.size.width = bsize.width;
	      rect.size.height = bsize.height;
	      [buttons[i] setFrame: rect];
	      position -= ButtonInterspace;
	    }
	}
    }

  // Finaly, place the message.
  if (useControl(messageField))
    {
      NSRect	mrect = [messageField frame];

      if (needsScroll)
	{
	  NSRect	srect;
	  float		width;

	  // The scroll view takes all the space that is available.
	  srect.origin.x = bounds.origin.x + MessageHorzMargin;
	  if (numberOfButtons > 0)
	    {
	      srect.origin.y = bounds.origin.y + ButtonBottom
		+ bsize.height + MessageVertMargin;
	    }
	  else
	    {
	      srect.origin.y = bounds.origin.y + MessageVertMargin;
	    }
	  srect.size.width = bounds.size.width - 2 * MessageHorzMargin;
	  srect.size.height = bounds.origin.y + bounds.size.height
	    + MessageTop - srect.origin.y;
	  [scroll setFrame: srect];
	  if (!useControl(scroll))
	    {
	      [content addSubview: scroll];
	    }

	  /* Adjust the message field's width again so that it does not
	     exceed the scroll view's visible rectangle and we do not
	     need a horizontal scroller. */
	  [messageField removeFromSuperview];
	  width =
	    [NSScrollView contentSizeForFrameSize: srect.size
			    hasHorizontalScroller: NO
			      hasVerticalScroller: YES
				       borderType: [scroll borderType]].width;
	  mrect.origin = NSZeroPoint;
	  mrect.size =
	    [[messageField attributedStringValue]
	      boundingRectWithSize: NSMakeSize(width, 1e6)
			   options: 0].size;
	  [messageField setFrame: mrect];
	  [scroll setDocumentView: messageField];
	}
      else
	{
	  float	vmargin;

	  /*
	   * We must center vertically the messageField because
	   * the window has a minimum size, thus may be greater
	   * than expected.
	   */
	  mrect.origin.x = (wsize.width - mrect.size.width)/2;
	  vmargin = bounds.size.height + LineBottom-mrect.size.height;
	  if (numberOfButtons > 0)
	    {
	      vmargin -= ButtonBottom + bsize.height;
	    }
	  vmargin/= 2.0; // if negative, it'll bite up and down.
	  mrect.origin.y = bounds.origin.y + vmargin;
	  if (numberOfButtons > 0)
	    {
	      mrect.origin.y += ButtonBottom + bsize.height;
	    }
	  [messageField setFrame: mrect];
	}
    }
  else if (useControl(scroll))
    {
      [scroll removeFromSuperview];
    }

  isGreen = NO;
  [content display];
}

- (void) buttonAction: (id)sender
{
  if (![self isActivePanel])
    {
      NSLog(@"alert panel buttonAction: when not in modal loop\n");
      return;
    }
  result = [sender tag];
  [NSApp stopModalWithCode: result];
}

- (NSInteger) result
{
  return result;
}

- (BOOL) isActivePanel
{
  return [NSApp modalWindow] == self;
}

- (NSInteger) runModal
{
  if (GSCurrentThread() != GSAppKitThread)
    {
      [self performSelectorOnMainThread: _cmd
			     withObject: nil
			  waitUntilDone: YES];
    }
  else
    {
      if (isGreen)
	{
	  [self sizePanelToFit];
	}
      [NSApp runModalForWindow: self];
      [self orderOut: self];
    }
  return result;
}

- (void) setTitleBar: (NSString*)titleBar
		icon: (NSImage*)icon
	       title: (NSString*)title
	     message: (NSString*)message
{
  NSView	*content = [self contentView];

  if (titleBar != nil)
    {
      [self setTitle: titleBar];
    }
  if (icon != nil)
    {
      [icoButton setImage: icon];
    }
  if (title == nil)
    {
      title = titleBar;	// Fall back to the same text as the title bar
    }
  setControl(content, titleField, title);
  if (useControl(scroll))
    { 
      // TODO: Remove the following line once NSView is corrected.
      [scroll setDocumentView: nil];
      [scroll removeFromSuperview];
      [messageField removeFromSuperview];
    }
  setControl(content, messageField, message);

  /* If the message contains a newline character then align the
   * message to the left side, as it is quite undesirable for a long
   * message to appear aligned in the center
   */
  if ([message rangeOfString: @"\n"].location != NSNotFound)
    {
      [messageField setAlignment: NSLeftTextAlignment];
    }
  else
    {
      [messageField setAlignment: NSCenterTextAlignment];
    }
}

- (void) setTitleBar: (NSString*)titleBar
		icon: (NSImage*)icon
	       title: (NSString*)title
	     message: (NSString*)message
		 def: (NSString*)defaultButton
		 alt: (NSString*)alternateButton
	       other: (NSString*)otherButton
{
  NSView *content = [self contentView];

  [self setTitleBar: titleBar icon: icon title: title message: message];
  setControl(content, defButton, defaultButton);
  setControl(content, altButton, alternateButton);
  setControl(content, othButton, otherButton);
  if (useControl(defButton))
    {
      [self makeFirstResponder: defButton];
    }
  else
    {
      [self makeFirstResponder: self];
    }
  if (useControl(altButton))
    {
      setKeyEquivalent(altButton);
    }
  if (useControl(othButton))
    {
      setKeyEquivalent(othButton);
    }

  /* a *working* nextKeyView chain:
     the trick is that the 3 buttons are not always used (displayed)
     so we have to set the nextKeyView *each* time.
     Maybe some optimisation in the logic of this block will be good,
     however it seems too risky for a (so) small reward
     */
  {
    BOOL ud, ua, uo;
    ud = useControl(defButton);
    ua = useControl(altButton);
    uo = useControl(othButton);
    
    if (ud)
      {
	if (uo)
	  [defButton setNextKeyView: othButton];
	else if (ua)
	  [defButton setNextKeyView: altButton];
	else
	  {
	    [defButton setPreviousKeyView: nil];
	    [defButton setNextKeyView: nil];
	  }
      }
    
    if (uo)
      {
	if (ua)
	  [othButton setNextKeyView: altButton];
	else if (ud)
	  [othButton setNextKeyView: defButton];
	else
	  {
	    [othButton setPreviousKeyView: nil];
	    [othButton setNextKeyView: nil];
	  }
      }

    if (ua)
      {
	if (ud)
	  [altButton setNextKeyView: defButton];
	else if (uo)
	  [altButton setNextKeyView: othButton];
	else
	  {
	    [altButton setPreviousKeyView: nil];
	    [altButton setNextKeyView: nil];
	  }
      }
  }
  [self sizePanelToFit];
  isGreen = YES;
  result = NSAlertErrorReturn; 	/* If no button was pressed	*/
}

- (void) setButtons: (NSArray *)buttons;
{
  NSView *content = [self contentView];
  NSUInteger count = [buttons count];

  setButton(content, defButton, count > 0 ? [buttons objectAtIndex: 0] : nil);
  setButton(content, altButton, count > 1 ? [buttons objectAtIndex: 1] : nil);
  setButton(content, othButton, count > 2 ? [buttons objectAtIndex: 2] : nil);
  if (useControl(defButton))
    {
      [self makeFirstResponder: defButton];
    }
  else
    {
      [self makeFirstResponder: self];
    }

  /* a *working* nextKeyView chain:
     the trick is that the 3 buttons are not always used (displayed)
     so we have to set the nextKeyView *each* time.
     */
  if (count > 2)
    {
      [defButton setNextKeyView: othButton];
      [othButton setNextKeyView: altButton];
      [altButton setNextKeyView: defButton];
    }
  else if (count > 1)
    {
      [defButton setNextKeyView: altButton];
      [altButton setNextKeyView: defButton];
    }
  else if (count > 0)
    {
      [defButton setPreviousKeyView: nil];
      [defButton setNextKeyView: nil];
    }

  [self sizePanelToFit];
  isGreen = YES;
  result = NSAlertErrorReturn; 	/* If no button was pressed	*/
}

@end /* GSAlertPanel */

@implementation GSAlertPanel (GMArchiverMethods)

// Reuse createObjectForModelUnarchiver: from super class

- (void) encodeWithModelArchiver: (GMArchiver*)archiver
{
  [super encodeWithModelArchiver: archiver];
  [archiver encodeSize: [self frame].size withName: @"OriginalSize"];
  [archiver encodeObject: defButton withName: @"DefaultButton"];
  [archiver encodeObject: altButton withName: @"AlternateButton"];
  [archiver encodeObject: othButton withName: @"OtherButton"];
  [archiver encodeObject: icoButton withName: @"IconButton"];
  [archiver encodeObject: messageField withName: @"MessageField"];
  [archiver encodeObject: titleField withName: @"TitleField"];
}

- (id) initWithModelUnarchiver: (GMUnarchiver*)unarchiver
{
  self = [super initWithModelUnarchiver: unarchiver];
  if (self != nil)
    {
      (void)[unarchiver decodeSizeWithName: @"OriginalSize"];
      defButton = RETAIN([unarchiver decodeObjectWithName: @"DefaultButton"]);
      altButton = RETAIN([unarchiver decodeObjectWithName: @"AlternateButton"]);
      othButton = RETAIN([unarchiver decodeObjectWithName: @"OtherButton"]);
      icoButton = RETAIN([unarchiver decodeObjectWithName: @"IconButton"]);
      messageField = RETAIN([unarchiver decodeObjectWithName: @"MessageField"]);
      titleField = RETAIN([unarchiver decodeObjectWithName: @"TitleField"]);
      scroll = makeScrollViewWithRect(NSMakeRect(0.0, 0.0, 80.0, 80.0));
      result = NSAlertErrorReturn;
      isGreen = YES;
    }
  return self;
}

@end /* GSAlertPanel GMArchiverMethods */

/*
  GSAlertSheet.  This class provides a borderless window which is
  attached to the parent window.
 */

@interface GSAlertSheet : GSAlertPanel
@end

@implementation GSAlertSheet
+ (void) initialize
{
  if (self == [GSAlertSheet class])
    {
      if (nc == nil)
	{
	  nc = [NSNotificationCenter defaultCenter];
	}
      [self setVersion: 0];
    }
}

- (id) initWithContentRect: (NSRect)contentRect
                 styleMask: (NSUInteger)aStyle
                   backing: (NSBackingStoreType)bufferingType
                     defer: (BOOL)flag
{
  if (NSIsEmptyRect(contentRect))
    {
      contentRect = NSMakeRect(0,0,100,100);
    }

  self = [super initWithContentRect: contentRect
		styleMask: NSBorderlessWindowMask
		backing: bufferingType
		defer: flag];
  if (self != nil)
    {
      // FIXME
    }
  return self;
}

- (NSRect) frameFromParentWindowFrame
{
  id parent = [self parentWindow];
  NSRect frame = [self frame];
  NSRect newFrame = NSZeroRect;  // return zero rect, if parent isn't set.
  
  if(parent != nil)
    {
      NSRect contentRect = [[parent contentView] frame];
      
      //
      // The calculation is based on the contentRect of the parent window
      // since we want the sheet to appear just inside of it.
      //
      newFrame = [parent frame];
      newFrame.origin.x += ((newFrame.size.width - frame.size.width) / 2); 
      newFrame.origin.y += (contentRect.size.height - frame.size.height) + 5;
    }

  return newFrame;
}

- (void) resetWindow
{
  NSRect frame = [self frameFromParentWindowFrame]; 
  NSWindow *parent = nil;

  if((parent = [self parentWindow]) != nil)
    {
      [self setBackgroundColor: 
	      [[parent backgroundColor] 
		highlightWithLevel: 0.4]];
    }

  [self setFrame: frame display: YES];
}

- (void) setParentWindow: (NSWindow *)window
{
  [super setParentWindow: window];
  [self resetWindow];
  /*
  [nc removeObserver: self];

  if (parent != nil)
    {
      // add observers....
      [nc addObserver: self
	selector: @selector(handleWindowClose:)
	name: NSWindowWillCloseNotification
	object: parent];
	
      [nc addObserver: self
	selector: @selector(handleWindowMiniaturize:)
	name: NSWindowWillMiniaturizeNotification
	object: parent];
	
      [nc addObserver: self
	selector: @selector(handleWindowMove:)
	name: NSWindowWillMoveNotification
	object: parent];
	
      [nc addObserver: self
	selector: @selector(handleWindowMove:)
	name: NSWindowDidResizeNotification
	object: parent];
	
      [nc addObserver: self
	selector: @selector(handleWindowDidBecomeKey:)
	name: NSWindowDidBecomeKeyNotification
	object: parent];
    }
  */
}

/*
- (void) handleWindowClose: (NSNotification *)notification
{
  [self close];
}

- (void) handleWindowMiniaturize: (NSNotification *)notification
{
  [self close];
}

- (void) handleWindowMove: (NSNotification *)notification
{
  [self _resetWindowPosition];
}

- (void) handleWindowDidBecomeKey: (NSNotification *)notification
{
  [self _resetWindowPosition];
}

- (void) dealloc
{
  [nc removeObserver: self];
  [super dealloc];
}
*/
@end

/*
  These functions may be called "recursively". For example, from a
  timed event. Therefore, there  may be several alert panel active
  at  the  same  time,  but   only  the  first  one  will  be  THE
  standardAlertPanel,  which will  not be  released  once finished
  with, but which will be kept for future use.

	   +---------+---------+---------+---------+---------+
	   | std !=0 | std act | pan=std | pan=new | std=new |
	   +---------+---------+---------+---------+---------+
     a:    |    F    |   N/A   |         |    X    |    X    |
	   +---------+---------+---------+---------+---------+
     b:    |    V    |    F    |    X    |         |         |
	   +---------+---------+---------+---------+---------+
     c:    |    V    |    V    |         |    X    |         |
	   +---------+---------+---------+---------+---------+
*/


/*
    TODO: Check if this discrepancy is wanted and needed.
          If not, we could merge these parameters, even
          for the alert panel, setting its window title to "Alert".
*/

@interface	_GSAlertCreation : NSObject
{
  GSAlertPanel **instance;
  NSString *defaultTitle;
  NSString *title;
  NSString *message;
  NSString *defaultButton;
  NSString *alternateButton;
  NSString *otherButton;
  GSAlertPanel *panel;
}
- (id) initWithInstance: (GSAlertPanel**)_instance
	   defaultTitle: (NSString*)_defaultTitle
		  title: (NSString*)_title
		message: (NSString*)_message
	  defaultButton: (NSString*)_defaultButton
	alternateButton: (NSString*)_alternateButton
	    otherButton: (NSString*)_otherButton;
- (void) makePanel;
- (void) makeSheet;
- (GSAlertPanel*) panel;
@end

@implementation	_GSAlertCreation
- (void) dealloc
{
  RELEASE(defaultTitle);
  RELEASE(title);
  RELEASE(defaultButton);
  RELEASE(alternateButton);
  RELEASE(otherButton);
  [super dealloc];
}

- (id) initWithInstance: (GSAlertPanel**)_instance
	   defaultTitle: (NSString*)_defaultTitle
		  title: (NSString*)_title
		message: (NSString*)_message
	  defaultButton: (NSString*)_defaultButton
	alternateButton: (NSString*)_alternateButton
	    otherButton: (NSString*)_otherButton
{
  instance = _instance;
  ASSIGNCOPY(defaultTitle, _defaultTitle);
  ASSIGNCOPY(title, _title);
  ASSIGNCOPY(message, _message);
  ASSIGNCOPY(defaultButton, _defaultButton);
  ASSIGNCOPY(alternateButton, _alternateButton);
  ASSIGNCOPY(otherButton, _otherButton);
  return self;
}

- (void) makePanel
{
  if (*instance != 0 && [*instance isMemberOfClass: [GSAlertPanel class]])
    {
      if ([*instance isActivePanel])
	{				// c:
	  panel = [[GSAlertPanel alloc] init];
	}
      else
	{				// b:
	  panel = *instance;
	}
    }
  else
    { 					// a:
      panel = [[GSAlertPanel alloc] init];
      *instance = panel;
    }

  [panel setTitleBar: defaultTitle
		icon: nil
	       title: title
	     message: message
		 def: defaultButton
		 alt: alternateButton
	       other: otherButton];
}

- (void) makeSheet
{
  if (*instance != 0 && [*instance isMemberOfClass: [GSAlertSheet class]])
    {
      if ([*instance isActivePanel])
	{				// c:
	  panel = [[GSAlertSheet alloc] init];
	}
      else
	{				// b:
	  panel = *instance;
	}
    }
  else
    { 					// a:
      panel = [[GSAlertSheet alloc] init];
      *instance = panel;
    }

  [panel setTitleBar: defaultTitle
		icon: nil
	       title: title
	     message: message
		 def: defaultButton
		 alt: alternateButton
	       other: otherButton];
}

- (GSAlertPanel*) panel
{
  return panel;
}
@end

static GSAlertPanel*
getSomePanel(
  GSAlertPanel **instance,
  NSString *defaultTitle,
  NSString *title,
  NSString *message,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton)
{
  GSAlertPanel	*panel;

  if (GSCurrentThread() != GSAppKitThread)
    {
      _GSAlertCreation	*c;

      NSWarnFLog(@"Alert Panel functionality called from a thread other than"
        @" the main one, this may not work on MacOS-X and could therefore be"
	@" a portability problem in your code");
      c = [_GSAlertCreation alloc];
      c = [c initWithInstance: instance
		 defaultTitle: defaultTitle
			title: title
		      message: message
		defaultButton: defaultButton
	      alternateButton: alternateButton
		  otherButton: otherButton];
      [c performSelectorOnMainThread: @selector(makePanel)
			  withObject: nil
		       waitUntilDone: YES];
      panel = [c panel];
      RELEASE(c);
    }
  else
    {
      if (*instance != 0 && [*instance isMemberOfClass: [GSAlertPanel class]])
	{
	  if ([*instance isActivePanel])
	    {				// c:
	      panel = [[GSAlertPanel alloc] init];
	    }
	  else
	    {				// b:
	      panel = *instance;
	    }
	}
      else
	{ 					// a:
	  panel = [[GSAlertPanel alloc] init];
	  *instance = panel;
	}

      [panel setTitleBar: defaultTitle
		    icon: nil
		   title: title
		 message: message
		     def: defaultButton
		     alt: alternateButton
		   other: otherButton];
    }
  return panel;
}

static GSAlertPanel*
getSomeSheet(
  GSAlertPanel **instance,
  NSString *defaultTitle,
  NSString *title,
  NSString *message,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton)
{
  GSAlertSheet	*panel;

  if (GSCurrentThread() != GSAppKitThread)
    {
      _GSAlertCreation	*c;

      NSWarnFLog(@"Alert Sheet functionality called from a thread other than"
        @" the main one, this may not work on MacOS-X and could therefore be"
	@" a portability problem in your code");
      c = [_GSAlertCreation alloc];
      c = [c initWithInstance: instance
		 defaultTitle: defaultTitle
			title: title
		      message: message
		defaultButton: defaultButton
	      alternateButton: alternateButton
		  otherButton: otherButton];
      [c performSelectorOnMainThread: @selector(makeSheet)
			  withObject: nil
		       waitUntilDone: YES];
      panel = (GSAlertSheet *)[c panel];
      RELEASE(c);
    }
  else
    {
      if (*instance != 0 && [*instance isMemberOfClass: [GSAlertSheet class]])
	{
	  if ([*instance isActivePanel])
	    {				// c:
	      panel = [[GSAlertSheet alloc] init];
	    }
	  else
	    {				// b:
	      panel = (GSAlertSheet *)*instance;
	    }
	}
      else
	{ 					// a:
	  panel = [[GSAlertSheet alloc] init];
	  *instance = panel;
	}

      [panel setTitleBar: defaultTitle
		    icon: nil
		   title: title
		 message: message
		     def: defaultButton
		     alt: alternateButton
		   other: otherButton];
    }
  return panel;
}

id
NSGetAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  return getSomePanel(&standardAlertPanel, defaultTitle, title, message,
    defaultButton, alternateButton, otherButton);
}

NSInteger
NSRunAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;
  GSAlertPanel	*panel;
  NSInteger	result;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  if (NSApp == nil)
    {
      // No NSApp ... not running in a gui application so just log.
      NSLog(@"%@", message);
      return NSAlertDefaultReturn;
    }
  if (defaultButton == nil)
    {
      defaultButton = @"OK";
    }

  panel = getSomePanel(&standardAlertPanel, defaultTitle, title, message,
    defaultButton, alternateButton, otherButton);
  result = [panel runModal];
  NSReleaseAlertPanel(panel);
  return result;
}

NSInteger
NSRunLocalizedAlertPanel(
  NSString *table,
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;
  GSAlertPanel	*panel;
  NSInteger	result;
  NSBundle	*bundle = [NSBundle mainBundle];

  if (title == nil)
    {
      title = defaultTitle;
    }

#define localize(string) if (string != nil) \
  string = [bundle localizedStringForKey: string value: string table: table]

  localize(title);
  localize(defaultButton);
  localize(alternateButton);
  localize(otherButton);
  localize(msg);

#undef localize

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  if (defaultButton == nil)
    {
      defaultButton = @"OK";
    }

  panel = getSomePanel(&standardAlertPanel, @"Alert", title, message,
		     defaultButton, alternateButton, otherButton);
  result = [panel runModal];
  NSReleaseAlertPanel(panel);
  return result;
}



id
NSGetCriticalAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  return getSomePanel(&criticalAlertPanel, @"Critical", title, message,
    defaultButton, alternateButton, otherButton);
}

NSInteger
NSRunCriticalAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;
  GSAlertPanel	*panel;
  NSInteger	result;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  panel = getSomePanel(&criticalAlertPanel, @"Critical", title, message,
    defaultButton, alternateButton, otherButton);
  result = [panel runModal];
  NSReleaseAlertPanel(panel);
  return result;
}


id
NSGetInformationalAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list	ap;
  NSString	*message;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  return getSomePanel(&informationalAlertPanel, @"Information", title, message,
    defaultButton, alternateButton, otherButton);
}


NSInteger
NSRunInformationalAlertPanel(
  NSString *title,
  NSString *msg,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton, ...)
{
  va_list       ap;
  NSString	*message;
  GSAlertPanel	*panel;
  NSInteger	result;

  va_start(ap, otherButton);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  panel = getSomePanel(&informationalAlertPanel,
		      @"Information",
		      title, message,
		      defaultButton, alternateButton, otherButton);
  result = [panel runModal];
  NSReleaseAlertPanel(panel);
  return result;
}

void
NSReleaseAlertPanel(id panel)
{
  if ((panel != standardAlertPanel)
    && (panel != informationalAlertPanel)
    && (panel != criticalAlertPanel))
    {
      RELEASE(panel);
    }
}

//
// New alert interface of Mac OS X
//
void NSBeginAlertSheet(NSString *title, 
		       NSString *defaultButton, 
		       NSString *alternateButton, 
		       NSString *otherButton, 
		       NSWindow *docWindow, 
		       id modalDelegate, 
		       SEL didEndSelector, 
		       SEL didDismissSelector, 
		       void *contextInfo, 
		       NSString *msg, ...)
{
  va_list	ap;
  NSString	*message;
  GSAlertPanel	*panel;

  va_start(ap, msg);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  if (defaultButton == nil)
    {
      defaultButton = @"OK";
    }

  panel = getSomeSheet(&standardAlertPanel, defaultTitle, title, message,
    defaultButton, alternateButton, otherButton);

  // FIXME: We should also change the button action to call endSheet:
  [NSApp beginSheet: panel
	 modalForWindow: docWindow
	 modalDelegate: modalDelegate
	 didEndSelector: didEndSelector
	 contextInfo: contextInfo];
  if (modalDelegate && [modalDelegate respondsToSelector: didDismissSelector])
    {
      void (*didDismiss)(id, SEL, id, NSInteger, void*);
      didDismiss = (void (*)(id, SEL, id, NSInteger, void*))[modalDelegate
        methodForSelector: didDismissSelector];
      didDismiss(modalDelegate, didDismissSelector, panel, [panel result],
        contextInfo);
    }

  NSReleaseAlertPanel(panel);
}

void NSBeginCriticalAlertSheet(NSString *title, 
			       NSString *defaultButton, 
			       NSString *alternateButton, 
			       NSString *otherButton, 
			       NSWindow *docWindow, 
			       id modalDelegate, 
			       SEL didEndSelector, 
			       SEL didDismissSelector, 
			       void *contextInfo, 
			       NSString *msg, ...)
{
  va_list	ap;
  NSString	*message;
  GSAlertPanel	*panel;

  va_start(ap, msg);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  panel = getSomeSheet(&criticalAlertPanel, @"Critical", title, message,
    defaultButton, alternateButton, otherButton);
  // FIXME: We should also change the button action to call endSheet:
  [NSApp beginSheet: panel
	 modalForWindow: docWindow
	 modalDelegate: modalDelegate
	 didEndSelector: didEndSelector
	 contextInfo: contextInfo];
  if (modalDelegate && [modalDelegate respondsToSelector: didDismissSelector])
    {
      void (*didDismiss)(id, SEL, id, NSInteger, void*);
      didDismiss = (void (*)(id, SEL, id, NSInteger, void*))[modalDelegate
        methodForSelector: didDismissSelector];
      didDismiss(modalDelegate, didDismissSelector, panel, [panel result],
        contextInfo);
    }

  NSReleaseAlertPanel(panel);
}

void NSBeginInformationalAlertSheet(NSString *title, 
				    NSString *defaultButton, 
				    NSString *alternateButton, 
				    NSString *otherButton,
				    NSWindow *docWindow, 
				    id modalDelegate, 
				    SEL didEndSelector, 
				    SEL didDismissSelector, 
				    void *contextInfo, 
				    NSString *msg, ...)
{
  va_list       ap;
  NSString	*message;
  GSAlertPanel	*panel;

  va_start(ap, msg);
  message = [NSString stringWithFormat: msg arguments: ap];
  va_end(ap);

  panel = getSomeSheet(&informationalAlertPanel,
		      @"Information",
		      title, message,
		      defaultButton, alternateButton, otherButton);
  // FIXME: We should also change the button action to call endSheet:
  [NSApp beginSheet: panel
	 modalForWindow: docWindow
	 modalDelegate: modalDelegate
	 didEndSelector: didEndSelector
	 contextInfo: contextInfo];
  if (modalDelegate && [modalDelegate respondsToSelector: didDismissSelector])
    {
      void (*didDismiss)(id, SEL, id, NSInteger, void*);
      didDismiss = (void (*)(id, SEL, id, NSInteger, void*))[modalDelegate
        methodForSelector: didDismissSelector];
      didDismiss(modalDelegate, didDismissSelector, panel, [panel result],
        contextInfo);
    }

  NSReleaseAlertPanel(panel);
}


@implementation	NSAlert

/*
 * Class methods
 */
+ (void) initialize
{
  if (self  ==  [NSAlert class])
    {
      [self setVersion: 1];
    }
}

+ (NSAlert *) alertWithError: (NSError *)error
{
  NSArray *options;
  NSUInteger count;
  NSString *errorText;

  errorText = [error localizedFailureReason];
  if (errorText == nil)
    {
      errorText = [error localizedDescription];
    }

  options = [error localizedRecoveryOptions];
  count = [options count];
  return [self alertWithMessageText: errorText
               defaultButton: (count > 0) ? [options objectAtIndex: 0] : nil
               alternateButton: (count > 1) ? [options objectAtIndex: 1] : nil
               otherButton: (count > 2) ? [options objectAtIndex: 2] : nil
               informativeTextWithFormat: [error localizedRecoverySuggestion]];
}

+ (NSAlert *) alertWithMessageText: (NSString *)messageTitle
		     defaultButton: (NSString *)defaultButtonTitle
		   alternateButton: (NSString *)alternateButtonTitle
		       otherButton: (NSString *)otherButtonTitle
	 informativeTextWithFormat: (NSString *)format, ...
{
  va_list ap;
  NSAlert *alert = [[self alloc] init];
  NSButton *but;
  NSString *text;

  va_start(ap, format);
  if (format != nil)
    {
      text = [[NSString alloc] initWithFormat: format arguments: ap];
      [alert setInformativeText: text];
      RELEASE(text);
    }
  va_end(ap);

  [alert setMessageText: messageTitle];

  if (defaultButtonTitle != nil)
    {
      but = [alert addButtonWithTitle: defaultButtonTitle];
    }
  else
    {
      but = [alert addButtonWithTitle: _(@"OK")];
    }
  [but setTag: NSAlertDefaultReturn];

  if (alternateButtonTitle != nil)
    {
      but = [alert addButtonWithTitle: alternateButtonTitle];
      [but setTag: NSAlertAlternateReturn];
    }

  if (otherButtonTitle != nil)
    {
      but = [alert addButtonWithTitle: otherButtonTitle];
      [but setTag: NSAlertOtherReturn];
    }

  return AUTORELEASE(alert);
}

- (id) init
{
  _buttons = [[NSMutableArray alloc] init];
  _style = NSWarningAlertStyle;
  return self;
} 

- (void) dealloc
{
  RELEASE(_informative_text);
  RELEASE(_message_text);
  RELEASE(_icon);
  RELEASE(_buttons);
  RELEASE(_help_anchor);
  RELEASE(_window);
  [super dealloc];
}

- (void) setInformativeText: (NSString *)informativeText
{
  ASSIGN(_informative_text, informativeText);
}

- (NSString *) informativeText
{
  return _informative_text;
}

- (void) setMessageText: (NSString *)messageText
{
  ASSIGN(_message_text, messageText);
}

- (NSString *) messageText
{
  return _message_text;
}

- (void) setIcon: (NSImage *)icon
{
  ASSIGN(_icon, icon);
}

- (NSImage *) icon
{
  return _icon;
}

- (NSButton *) addButtonWithTitle: (NSString *)aTitle
{
  NSButton *button = [[NSButton alloc] init];
  NSUInteger count = [_buttons count];

  [button setTitle: aTitle];
  [button setAutoresizingMask: NSViewMinXMargin | NSViewMaxYMargin];
  [button setButtonType: NSMomentaryPushButton];
  [button setTarget: self];
  [button setAction: @selector(buttonAction:)];
  [button setFont: [NSFont systemFontOfSize: 0]];
  if (count == 0)
    {
      [button setTag: NSAlertFirstButtonReturn];
      [button setKeyEquivalent: @"\r"];
    }
  else
    {
      [button setTag: NSAlertFirstButtonReturn + count];
      setKeyEquivalent(button);
    }

  [_buttons addObject: button];
  RELEASE(button);
  return button;
}

- (NSArray *) buttons
{
  return _buttons;
}

- (void) setShowsHelp: (BOOL)showsHelp
{
  _shows_help = showsHelp;
}

- (BOOL) showsHelp
{
  return _shows_help;
}

- (void) setHelpAnchor: (NSString *)anchor
{
  ASSIGN(_help_anchor, anchor);
}

- (NSString *) helpAnchor
{
  return _help_anchor;
}

- (void) setAlertStyle: (NSAlertStyle)style
{
  _style = style;
}

- (NSAlertStyle) alertStyle
{
  return _style;
}

- (void) setDelegate: (id)delegate
{
  _delegate = delegate;
}

- (id) delegate
{
  return _delegate;
}

- (void) _setupPanel
{
  if (GSCurrentThread() != GSAppKitThread)
    {
      [self performSelectorOnMainThread: _cmd
			     withObject: nil
			  waitUntilDone: YES];
    }
  else
    {
      GSAlertPanel *panel;
      NSString *title;

      panel = [[GSAlertPanel alloc] init];
      _window = panel;

      switch (_style)
        {
          case NSCriticalAlertStyle: 
            title = @"Critical";
            break;
          case NSInformationalAlertStyle: 
            title = @"Information";
            break;
          case NSWarningAlertStyle:
          default:
            title = @"Alert";
            break;
        }
      [panel setTitleBar: title
             icon: _icon
             title: _message_text != nil ? _message_text : _(@"Alert")
	     message: _informative_text != nil ? _informative_text : _(@"No information")];
      if ([_buttons count] == 0)
        {
          [self addButtonWithTitle: @"OK"];
        }
      [panel setButtons: _buttons];
    }
}

- (NSInteger) runModal
{
  if (GSCurrentThread() != GSAppKitThread)
    {
      [self performSelectorOnMainThread: _cmd
			     withObject: nil
			  waitUntilDone: YES];
      return _result;
    }
  else
    {
      [self _setupPanel];
      [NSApp runModalForWindow: _window];
      [_window orderOut: self];
      _result = [(GSAlertPanel*)_window result];
      DESTROY(_window);
      return _result;
    }
}

- (void) beginSheetModalForWindow: (NSWindow *)window
		    modalDelegate: (id)delegate
		   didEndSelector: (SEL)didEndSelector
		      contextInfo: (void *)contextInfo
{
  [self _setupPanel];
  _modalDelegate = delegate;
  _didEndSelector = didEndSelector;
  [NSApp beginSheet: _window
         modalForWindow: window
         modalDelegate: self
         didEndSelector: @selector(_alertDidEnd:returnCode:contextInfo:)
         contextInfo: contextInfo];
  DESTROY(_window);
}

- (void) _alertDidEnd: (NSWindow *)sheet
           returnCode: (NSInteger)returnCode
	  contextInfo: (void *)contextInfo
{
  if ([_modalDelegate respondsToSelector: _didEndSelector])
    {
      void (*didEnd)(id, SEL, id, NSInteger, void *);
      didEnd = (void (*)(id, SEL, id, NSInteger, void *))[_modalDelegate
	methodForSelector: _didEndSelector];
      didEnd(_modalDelegate, _didEndSelector, self, returnCode, contextInfo);
    }
}

- (id) window
{
  return _window;
}

@end

@interface GSExceptionPanel : GSAlertPanel
{
  NSBrowser *_browser;
  NSDictionary *_userInfo;
  NSPanel *_userInfoPanel;
}
- (void) setUserInfo: (NSDictionary *)userInfo;
- (NSPanel *) userInfoPanel;
@end

NSInteger GSRunExceptionPanel(
  NSString *title,
  NSException *exception,
  NSString *defaultButton,
  NSString *alternateButton,
  NSString *otherButton)
{
  NSString      *message;
  GSExceptionPanel  *panel;
  NSInteger      result;

  message = [NSString stringWithFormat: @"%@: %@",
	  			[exception name],
				[exception reason]];
  if (defaultButton == nil)
    {
      defaultButton = @"OK";
    }

  panel = [[GSExceptionPanel alloc] init];

  if (title == nil)
    {
      title = @"Exception";
    }

  [panel setTitleBar: nil
  		icon: nil
	       title: title
	     message: message
		 def: defaultButton
		 alt: alternateButton
	       other: otherButton];
  [panel setUserInfo: [exception userInfo]];
  result = [panel runModal];
  [[panel userInfoPanel] orderOut: nil];
  [panel setUserInfo: nil];
  
  RELEASE(panel);
  return result;
}

@implementation GSExceptionPanel
- (void) dealloc
{
  RELEASE(_userInfo);
  RELEASE(_browser);
  RELEASE(_userInfoPanel);
  [super dealloc];
}

- (id) init
{
  if ((self = [super init]))
    {
      [icoButton setEnabled: YES];
      [icoButton setTarget: self];
      [icoButton setAction: @selector(_icoAction:)];
    }

  return self;
}

- (NSPanel *) userInfoPanel
{
  return _userInfoPanel;
}

- (void) setUserInfo: (NSDictionary *)userInfo;
{
  ASSIGN(_userInfo, userInfo);
  [_browser reloadColumn: 0];
}

- (void) _icoAction: (id)sender
{
  NSRect fr;
   
  if (_userInfoPanel) 
    {
      [_browser reloadColumn: 0];
      return;
    }

  fr = NSMakeRect(_frame.origin.x, _frame.origin.y + _frame.size.height + 15, 
    _frame.size.width, 108);
  _userInfoPanel = [[NSPanel alloc] initWithContentRect: fr
    styleMask: NSTitledWindowMask | NSResizableWindowMask
    backing: NSBackingStoreBuffered
    defer: NO];
  [_userInfoPanel setTitle: @"User Info Inspector"];
  [_userInfoPanel setWorksWhenModal: YES];
  
  fr = NSMakeRect(8, 8, _frame.size.width - 16, 100);
  _browser = [[NSBrowser alloc] initWithFrame: fr];
  [_browser setMaxVisibleColumns: 2];
  [_browser setDelegate: self];
  [_browser setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [_browser reloadColumn: 0];
  [[_userInfoPanel contentView] addSubview:_browser];
  [_userInfoPanel makeKeyAndOrderFront: self];
}

- (NSInteger) browser: (id)browser
numberOfRowsInColumn: (NSInteger)col
{
  if (col == 0)
    return [[_userInfo allKeys] count];
  else
    {
      id val;
      volatile id foo = nil;

      val = [[(NSCell *)[browser selectedCellInColumn: col - 1]
	representedObject] description];
      NS_DURING
         foo = [val propertyList];
         val = foo;
      NS_HANDLER
      NS_ENDHANDLER
      
      if ([val isKindOfClass: [NSArray class]])
	return [val count];
      else if ([val isKindOfClass: [NSDictionary class]])
        return [[val allKeys] count];
      else return val != nil;
    }
  return 0;
}

- (void) browser: (NSBrowser *)browser
  willDisplayCell: (NSBrowserCell *)cell
  atRow: (NSInteger)row
  column: (NSInteger)column
{
  if (column == 0)
    {
      id key = [[_userInfo allKeys] objectAtIndex: row]; 
      id val = [_userInfo objectForKey: key]; 

      [cell setLeaf: NO];
      [cell setStringValue: [key description]];
      [cell setRepresentedObject: val];
    }
  else
    {
      volatile id val;
      BOOL flag;
     
      val = [(NSCell *)[browser selectedCellInColumn: column - 1]
	representedObject];
      if (!([val isKindOfClass: [NSArray class]]
	|| [val isKindOfClass: [NSArray class]]))
        {
          volatile id foo = nil;
	  val = [val description];
	  NS_DURING 
	    foo = [val propertyList];
	    val = foo;
	  NS_HANDLER
	  NS_ENDHANDLER
	}    
      flag = (!([val isKindOfClass: [NSArray class]]
	|| [val isKindOfClass: [NSDictionary class]]));
     
      [cell setLeaf: flag];
      
      if ([val isKindOfClass: [NSArray class]])
        {
	  volatile id obj = [val objectAtIndex: row];

	  if (!([obj isKindOfClass: [NSArray class]]
	    || [obj isKindOfClass: [NSArray class]]))
	    {
	      volatile id foo;
	      obj = [[obj description] propertyList]; 
	      NS_DURING
	        foo = [obj propertyList]; 
	        obj = foo;
	      NS_HANDLER
	      NS_ENDHANDLER
	    }

	  if ([obj isKindOfClass: [NSArray class]])
	    {
              [cell setRepresentedObject: obj];
	      [cell setLeaf: NO];
              [cell setStringValue:
		[NSString stringWithFormat: @"%@ %p", [obj class], obj]];
	    }
	  else if ([obj isKindOfClass: [NSDictionary class]])
	    {
	      [cell setRepresentedObject: obj];
	      [cell setLeaf: NO];
              [cell setStringValue:
		[NSString stringWithFormat: @"%@ %p", [obj class], obj]];
	    }
	  else
	    {
	      [cell setLeaf: YES];
	      [cell setStringValue: [obj description]];
	      [cell setRepresentedObject: nil];
	    }
	}
      else if ([val isKindOfClass: [NSDictionary class]])
        {
	  id key = [[val allKeys] objectAtIndex: row];
          volatile id it = [(NSDictionary *)val objectForKey: key];
	  volatile id foo;
	  foo = [it description];
	  NS_DURING
	    foo = [it propertyList];
	    it = foo;
	  NS_HANDLER
	  NS_ENDHANDLER
	  [cell setStringValue: [key description]];
	  [cell setRepresentedObject: it];
        } 
      else
        {
	  [cell setLeaf: YES];
	  [cell setStringValue: [val description]];
        }
    }
}

- (id) browser: (NSBrowser *)browser titleOfColumn: (NSInteger)column
{
  id val;
  NSString *title;

  if (column == 0)
    return @"userInfo";
  val = [(NSCell *)[browser selectedCellInColumn: column - 1]
    representedObject];
  title = [NSString stringWithFormat: @"%@ %p", [val class], val];
  return title;
}
@end

