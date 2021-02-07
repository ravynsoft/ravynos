/* 
   NSPrintOperation.m

   Controls operations generating print jobs.

   Copyright (C) 1996,2004 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: November 2000
   Started implementation.
   Modified for Printing Backend Support
   Author: Chad Hardin <cehardin@mac.com>
   Date: June 2004

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

#include <limits.h>
#include <math.h>
#include "config.h"
#import <Foundation/NSString.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSData.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSException.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSView.h"
#import "AppKit/NSPrinter.h"
#import "AppKit/NSPrintPanel.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSEPSPrintOperation.h"
#import "GNUstepGUI/GSPDFPrintOperation.h"
#import "GNUstepGUI/GSPrintOperation.h"
#import "GSGuiPrivate.h"

#define NSNUMBER(a) [NSNumber numberWithInt: (a)]
#define NSFNUMBER(a) [NSNumber numberWithDouble: (a)]

/*
 * When a view gets printed it may need to be split up into segments, if the 
 * views printed rectangle (after scaling) is bigger than the used area on the page.
 * In this case we set xpages and ypages to the number of segments needed per 
 * dimension. This pre-calculated value may not be accurate, as the view may 
 * adjust the rect for each printed page.
 * An independent concept is that multuple pages may be put on one sheet of paper.
 * This is taken care of by nup and nupScale. Here we currently only allow even 
 * values (or 1, when we don't use multiple pages). If we ever change this be 
 * sure to change [NSView beginPageInRect:atPlacement:], perhaps by moving that 
 * code to here?
 * We always end up printing two rows per page, this is fine for 2, 4, 6, 8 and 10, 
 * but starting from there it would be better to use three or four rows.
 */
/* Local pagination variables needed while printing */
typedef struct _page_info_t {
  NSRect scaledBounds;       /* View's rect scaled by the user specified scale
                                and page fitting */
  NSRect paperBounds;        /* Print area of a page in default user space, possibly
                                rotated if printing Landscape */
  NSRect sheetBounds;        /* Print area of a sheet in default user space */
  NSSize paperSize;          /* Size of the paper */
  int xpages, ypages;        /* number of page segments for the view in both dimensions */
  int first, last;           /* first and last page to print */
  double pageScale;          /* Scaling determined from page fitting */
  double printScale;         /* User specified scaling */
  double nupScale;           /* Scale required to fit nup pages on the sheet */
  int    nup;                /* Number of pages to print on a sheet */
  double lastWidth, lastHeight; /* max. values of last printed page (scaled) */
  NSPrintingOrientation orient;
  int    pageDirection;      /* NSPrintPageDirection */
} page_info_t;


@interface NSPrintOperation (TrulyPrivate)
- (BOOL) _runOperation;
- (void) _setupPrintInfo;
- (void)_printOperationDidRun:(NSPrintOperation *)printOperation 
                   returnCode:(int)returnCode
                  contextInfo:(void *)contextInfo;
- (void) _printPaginateWithInfo: (page_info_t *)info 
                     knowsRange: (BOOL)knowsRange;
- (NSRect) _rectForPage: (int)page info: (page_info_t *)info 
                  xpage: (int *)xptr
                  ypage: (int *)yptr;
- (NSRect) _adjustPagesFirst: (int)first 
                        last: (int)last
                        info: (page_info_t *)info;
- (void) _print;
@end






@interface NSView (NSPrintOperation)
- (void) _displayPageInRect: (NSRect)pageRect
                   withInfo: (page_info_t)info
	     knowsPageRange: (BOOL)knowsPageRange;
@end

@interface NSView (NPrintOperationPrivate)
- (void) _endSheet;
- (void) _cleanupPrinting;
@end



static NSString *NSPrintOperationThreadKey = @"NSPrintOperationThreadKey";

/**
  <unit>
  <heading>Class Description</heading>
  <p>
  NSPrintOperation controls printing of an NSView. When invoked normally
  it will (optionally) display a standard print panel (NSPrintPanel), and
  based on the information entered by the user here as well as information
  about page layout (see NSPageLayout) tells the NSView to print it's 
  contents. NSPrintOperation works with the NSView to paginate the output
  into appropriately sized and oriented pages and finally delivers the result
  to the appropriate place, whether it be a printer, and PostScript file,
  or another output.
  </p>
  </unit>
*/ 
@implementation NSPrintOperation

//
// Class methods
//
+ (void)initialize
{
  if (self == [NSPrintOperation class])
    {
      // Initial version
      [self setVersion:1];
    }
}

//
// Creating and Initializing an NSPrintOperation Object
//
+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView
                                insideRect:(NSRect)rect
                                    toData:(NSMutableData *)data
{
  return [self EPSOperationWithView: aView        
               insideRect: rect
               toData: data
               printInfo: nil];
}

+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView        
                                insideRect:(NSRect)rect
                                    toData:(NSMutableData *)data
                                 printInfo:(NSPrintInfo *)aPrintInfo
{
  return AUTORELEASE([[GSEPSPrintOperation alloc] initWithView: aView
                                                  insideRect: rect
                                                  toData: data
                                                  printInfo: aPrintInfo]);
}

+ (NSPrintOperation *)EPSOperationWithView:(NSView *)aView        
                                insideRect:(NSRect)rect
                                    toPath:(NSString *)path
                                 printInfo:(NSPrintInfo *)aPrintInfo
{
  return AUTORELEASE([[GSEPSPrintOperation alloc] initWithView: aView        
                                                  insideRect: rect
                                                  toPath: path
                                                  printInfo: aPrintInfo]);
}

+ (NSPrintOperation *)printOperationWithView:(NSView *)aView
{
  return [self printOperationWithView: aView
               printInfo: nil];
}

+ (NSPrintOperation *)printOperationWithView:(NSView *)aView
                                   printInfo:(NSPrintInfo *)aPrintInfo
{
  return AUTORELEASE([[GSPrintOperation alloc] initWithView: aView
                                               printInfo: aPrintInfo]);
}

+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toData:(NSMutableData *)data
{
  return [self PDFOperationWithView: aView 
               insideRect: rect 
               toData: data 
               printInfo: nil];
}

+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toData:(NSMutableData *)data 
                                 printInfo:(NSPrintInfo*)aPrintInfo
{
  return AUTORELEASE([[GSPDFPrintOperation alloc] 
                         initWithView: aView 
                         insideRect: rect 
                         toData: data 
                         printInfo: aPrintInfo]);
}

+ (NSPrintOperation *)PDFOperationWithView:(NSView *)aView 
                                insideRect:(NSRect)rect 
                                    toPath:(NSString *)path 
                                 printInfo:(NSPrintInfo*)aPrintInfo
{
  return AUTORELEASE([[GSPDFPrintOperation alloc] 
                         initWithView: aView 
                         insideRect: rect 
                         toPath: path 
                         printInfo: aPrintInfo]);
}

//
// Setting the Print Operation
//
/** Returns the NSPrintOperation object that is currently performing
    a print operation (if any).
*/
+ (NSPrintOperation *)currentOperation
{
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];

  return (NSPrintOperation*)[dict objectForKey: NSPrintOperationThreadKey];
}

/** Set the current NSPrintOperation to the supplied operation
    object. As this is currently implemented, if a NSPrintOperation
    is currently running, that operation is lost (along with any
    associated context), so be careful to call this only when there is
    no current operation.
*/
+ (void)setCurrentOperation:(NSPrintOperation *)operation
{
  NSMutableDictionary *dict = [[NSThread currentThread] threadDictionary];

  if (operation == nil)
    [dict removeObjectForKey: NSPrintOperationThreadKey];
  else
    [dict setObject: operation forKey: NSPrintOperationThreadKey];
}

//
// Instance methods
//
//
// Creating and Initializing an NSPrintOperation Object
//

- (id)initEPSOperationWithView:(NSView *)aView
                    insideRect:(NSRect)rect
                        toData:(NSMutableData *)data
                     printInfo:(NSPrintInfo *)aPrintInfo
{
  RELEASE(self);
  
  return [[GSEPSPrintOperation alloc] initWithView: aView        
                                      insideRect: rect
                                      toData: data
                                      printInfo: aPrintInfo];
}

- (id)initWithView:(NSView *)aView
         printInfo:(NSPrintInfo *)aPrintInfo
{
  RELEASE(self);
  
  return [[GSPrintOperation alloc] initWithView: aView
                                   printInfo: aPrintInfo];
}

- (void) dealloc
{
  RELEASE(_print_info);
  RELEASE(_view);  
  RELEASE(_data);
  TEST_RELEASE(_context);
  TEST_RELEASE(_print_panel);  
  TEST_RELEASE(_accessory_view);  
  TEST_RELEASE(_path);  
  TEST_RELEASE(_job_style_hint);  

  [super dealloc];
}

//
// Determining the Type of Operation
//
/** Returns YES if the receiver is performing an operation whose output
    is EPS format.
*/
- (BOOL)isEPSOperation
{
  return NO;
}

- (BOOL)isCopyingOperation
{
  return NO;
}

//
// Controlling the User Interface
//
/** Returns the NSPrintPanel associated with the receiver.
 */
- (NSPrintPanel *)printPanel
{
  if (_print_panel == nil)
    ASSIGN(_print_panel, [NSPrintPanel printPanel]); 

  return _print_panel;
}

/** Returns YES if the reciever display an NSPrintPanel and other information
    when running a print operation. */
- (BOOL)showPanels
{
  return [self showsPrintPanel] && [self showsProgressPanel];
}

/** Sets the NSPrintPanel used by the receiver obtaining and displaying
    printing information from/to the user.
*/
- (void)setPrintPanel:(NSPrintPanel *)panel
{
  ASSIGN(_print_panel, panel);
}

/** Use this to set whether a print panel is displayed during a printing
    operation. If set to NO, then the receiver uses information that
    was previously set and does not display any status information about the
    progress of the printing operation.
*/
- (void)setShowPanels:(BOOL)flag
{
  [self setShowsPrintPanel: flag];
  [self setShowsProgressPanel: flag];
}

- (BOOL)showsPrintPanel
{
  return _flags.show_print_panel;
}

- (void)setShowsPrintPanel:(BOOL)flag
{
  _flags.show_print_panel = flag;
}

- (BOOL)showsProgressPanel
{
  return _flags.show_progress_panel;
}

- (void)setShowsProgressPanel:(BOOL)flag
{
  _flags.show_progress_panel = flag;
}


/** Returns the accessory view used by the NSPrintPanel associated with
    the receiver.
*/
- (NSView *)accessoryView
{
  return _accessory_view;
}

/** Set the accessory view used by the NSPrintPanel associated with the
    receiver.
*/
- (void)setAccessoryView:(NSView *)aView
{
  ASSIGN(_accessory_view, aView);
}

//
// Managing the drawing Context
//
/** This method is used by the print operation to create a special
   graphics context for use while running the print operation.
*/
- (NSGraphicsContext*)createContext
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/** Returns the graphic contexts used by the print operation.
*/
- (NSGraphicsContext *)context
{
  return _context;
}

/** This method is used by the print operation to destroy the special
    graphic context used while running the print operation.
*/
- (void)destroyContext
{
  DESTROY(_context);
}

//
// Page Information
//
/** Returns the page currently being printing. Returns 0 if no page
    is currently being printed
*/
- (int)currentPage
{
  return _currentPage;
}

/** Returns the page order of printing.
*/
- (NSPrintingPageOrder)pageOrder
{
  return _page_order;
}

/** Set the page order used when printing.
 */
- (void)setPageOrder:(NSPrintingPageOrder)order
{
  _page_order = order;
}

//
// Running a Print Operation
//
/** Called by the print operation after it has finished running a printing
    operation.
*/
- (void)cleanUpOperation
{
  [[self printPanel] orderOut: self];
  _currentPage = 0;
  [NSPrintOperation setCurrentOperation: nil];
}

/** Called by the print operation to deliver the results of the printing
    operation. This might include sending the output to a printer, a file
    or a previewing program. Returns YES if the output was delivered 
    sucessfully.
*/
- (BOOL)deliverResult
{
  return NO;
}


/** Call this message to run the print operation on a view. This includes
    (optionally) displaying a print panel and working with the NSView to
    paginate and draw the contents of the view.
*/
- (BOOL)runOperation
{
  BOOL result;

  if ([self showsPrintPanel])
    {
      NSPrintPanel *panel = [self printPanel];
      int button;
      
      [panel setAccessoryView: _accessory_view];
      [self _setupPrintInfo];
      button = [panel runModalWithPrintInfo: _print_info];
      [panel setAccessoryView: nil];

      if (button != NSOKButton)
        {
          [self cleanUpOperation];
          return NO;
        }
    }

  result = NO;
  if ([self _runOperation])
    result = [self deliverResult];
  [self cleanUpOperation];

  return result;
}



/** Run a print operation modally with respect to a window.
 */
- (void)runOperationModalForWindow: (NSWindow *)docWindow 
                          delegate: (id)delegate 
                    didRunSelector: (SEL)didRunSelector 
                       contextInfo:(void *)contextInfo
{
  NSMutableDictionary *dict;
  NSPrintPanel *panel = [self printPanel];

  if (delegate != nil && didRunSelector != NULL)
    {
      /* Save the selector so we can use it later */
      dict = [_print_info dictionary];
      [dict setObject: [NSValue value: &didRunSelector withObjCType: @encode(SEL)]
               forKey: @"GSModalRunSelector"];
      [dict setObject: delegate
               forKey: @"GSModalRunDelegate"];
    }

  /* Assume we want to show the panel regardless of the value
     of _showPanels 
  */
  [panel setAccessoryView: _accessory_view];
  [self _setupPrintInfo];
  [panel beginSheetWithPrintInfo: _print_info 
                  modalForWindow: docWindow 
                        delegate: self 
                  didEndSelector: 
                          @selector(_printOperationDidRun:returnCode:contextInfo:)
                      contextInfo: contextInfo];
  [panel setAccessoryView: nil];
}

- (BOOL)canSpawnSeparateThread
{
  return _flags.can_spawn_separate_thread;
}

- (void)setCanSpawnSeparateThread:(BOOL)flag
{
  _flags.can_spawn_separate_thread = flag;
}

- (NSString *) jobStyleHint
{
  return _job_style_hint;
}

- (void)setJobStyleHint:(NSString *)hint
{
  ASSIGN(_job_style_hint, hint);
}

//
// Getting the NSPrintInfo Object
//
/** Returns the NSPrintInfo object associated with the receiver.
*/
- (NSPrintInfo *)printInfo
{
  return _print_info;
}

/** Set the NSPrintInfo object associated with the receiver.
 */
- (void)setPrintInfo:(NSPrintInfo *)aPrintInfo
{
  if (aPrintInfo == nil)
    aPrintInfo = [NSPrintInfo sharedPrintInfo];

  ASSIGNCOPY(_print_info, aPrintInfo);
}

//
// Getting the NSView Object
//
/** Return the view that is the being printed.
*/
- (NSView *)view
{
  return _view;
}

@end


@implementation NSPrintOperation (Private)

- (id) initWithView:(NSView *)aView
         insideRect:(NSRect)rect
             toData:(NSMutableData *)data
          printInfo:(NSPrintInfo *)aPrintInfo
{
  if ([NSPrintOperation currentOperation] != nil)
    {
      [NSException raise: NSPrintOperationExistsException
                   format: @"There is already a printoperation for this thread"];
    }

  ASSIGN(_view, aView);
  _rect = rect;
  ASSIGN(_data, data);
  _page_order = NSUnknownPageOrder;
  [self setShowsPrintPanel: NO];
  [self setShowsProgressPanel: NO];
  [self setPrintInfo: aPrintInfo];
  _path = nil;
  _currentPage = 0;

  [NSPrintOperation setCurrentOperation: self];
  return self;
}
@end

@implementation NSPrintOperation (TrulyPrivate)


/* Private method to run the printing operation. Needs to create an
   autoreleaes pool to make sure the print context is destroyed before
   returning (which closes the print file.) */
- (BOOL) _runOperation
{
  BOOL result;
  CREATE_AUTORELEASE_POOL(pool);
  NSGraphicsContext *oldContext = [NSGraphicsContext currentContext];

  [self createContext];
  if (_context == nil)
    return NO;

  result = NO;
  if (_page_order == NSUnknownPageOrder)
    {
      if ([[[_print_info dictionary] objectForKey: NSPrintReversePageOrder] 
            boolValue] == YES)
        _page_order = NSDescendingPageOrder;
      else
        _page_order = NSAscendingPageOrder;
    }

  [NSGraphicsContext setCurrentContext: _context];
  NS_DURING
    {
      [self _print];
      result = YES;
      [NSGraphicsContext setCurrentContext: oldContext];
    }
  NS_HANDLER
    {
      [_view _cleanupPrinting];
      [NSGraphicsContext setCurrentContext: oldContext];
      NSRunAlertPanel(_(@"Error"), _(@"Printing error: %@"), 
                      _(@"OK"), NULL, NULL, localException);
    }
  NS_ENDHANDLER
  [self destroyContext];
  RELEASE(pool);
  return result;
}

- (void) _setupPrintInfo
{
  BOOL knowsPageRange;
  NSRange viewPageRange = NSMakeRange(1, 0);
  NSMutableDictionary *dict = [_print_info dictionary];

  knowsPageRange = [_view knowsPageRange: &viewPageRange]; 
  if (knowsPageRange == YES)
    {
      int first = viewPageRange.location;
      int last = NSMaxRange(viewPageRange) - 1;
      [dict setObject: NSNUMBER(first) forKey: NSPrintFirstPage];
      [dict setObject: NSNUMBER(last) forKey: NSPrintLastPage];
    }
}

- (void)_printOperationDidRun:(NSPrintOperation *)printOperation 
                   returnCode:(int)returnCode  
                  contextInfo:(void *)contextInfo
{
  id delegate;
  SEL didRunSelector;
  BOOL success = NO;
  NSMutableDictionary *dict;
  void (*didRun)(id, SEL, BOOL, id);

  if (returnCode == NSOKButton)
    {
      if ([self _runOperation])
        success = [self deliverResult];
    }
  [self cleanUpOperation];
  dict = [_print_info dictionary];
  [[dict objectForKey: @"GSModalRunSelector"] getValue: &didRunSelector];
  delegate = [dict objectForKey: @"GSModalRunDelegate"];
  if (delegate != nil && didRunSelector != NULL)
    {
      didRun = (void (*)(id, SEL, BOOL, id))[delegate methodForSelector: 
                                                          didRunSelector];
      didRun (delegate, didRunSelector, success, contextInfo);
    }
}




/*
static NSSize
scaleSize(NSSize size, double scale)
{
  size.height *= scale;
  size.width  *= scale;
  return size;
}
*/

static NSRect
scaleRect(NSRect rect, double scale)
{
  return NSMakeRect(NSMinX(rect) * scale,
                    NSMinY(rect) * scale,
                    NSWidth(rect) * scale,
                    NSHeight(rect) * scale);
}

/* 
 * Pagination - guess how many pages we need to print. This could be off
 * in both X and Y because of the view's ability to adjust the width and 
 * height of the printRect during printing. 
 * Also set up a bunch of other information needed for printing.
 */
- (void) _printPaginateWithInfo: (page_info_t *)info knowsRange: (BOOL)knowsRange
{
  NSMutableDictionary *dict;
  NSNumber *value;

  dict = [_print_info dictionary];
  info->paperSize = [_print_info paperSize];
  info->orient = [_print_info orientation];
  value = [dict objectForKey: NSPrintScalingFactor];
  if (value != nil)
    info->printScale = [value doubleValue];
  else
    info->printScale = 1.0;

  info->nup = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
  info->nupScale = 1;
  if (info->nup < 1 || (info->nup > 1 && (((info->nup) & 0x1) == 1)))
    {
      /* Bad nup value */
      info->nup = 1;
      [dict setObject: NSNUMBER(1) forKey: NSPrintPagesPerSheet];
    }

  /* Subtract the margins from the paper size to get print boundary */
  info->paperBounds.size = info->paperSize;
  info->paperBounds.origin.x = [_print_info leftMargin];
  info->paperBounds.origin.y = [_print_info bottomMargin];
  info->paperBounds.size.width -= 
    ([_print_info rightMargin]+[_print_info leftMargin]);
  info->paperBounds.size.height -= 
    ([_print_info topMargin]+[_print_info bottomMargin]);

  if (info->orient == NSLandscapeOrientation)
    {
      /* Bounding box needs to be in default user space, but the bbox
         we get is rotated */
      info->sheetBounds = NSMakeRect(NSMinY(info->paperBounds), 
                                     NSMinX(info->paperBounds), 
                                     NSHeight(info->paperBounds), 
                                     NSWidth(info->paperBounds));
    }
  else
    {
      info->sheetBounds = info->paperBounds;
    }

  /* Save this for the view to look at */
  [dict setObject: [NSValue valueWithRect: info->paperBounds]
            forKey: @"NSPrintPaperBounds"];
  [dict setObject: [NSValue valueWithRect: info->sheetBounds]
            forKey: @"NSPrintSheetBounds"];

   /* Scale bounds by the user specified scaling */
  info->scaledBounds = scaleRect(_rect, info->printScale);
  info->pageScale = 1; // default

  if (knowsRange == NO)
    {
      /* Now calculate page fitting to get page scale */
      if ([_print_info horizontalPagination] == NSFitPagination)
        info->pageScale  = info->paperBounds.size.width 
          / NSWidth(info->scaledBounds);
      if ([_print_info verticalPagination] == NSFitPagination)
        info->pageScale = MIN(info->pageScale,
          NSHeight(info->paperBounds)/NSHeight(info->scaledBounds));
      /* Scale bounds by pageScale */
      info->scaledBounds = scaleRect(info->scaledBounds, info->pageScale);

      /* Now find out how many pages */
      info->xpages = ceil(NSWidth(info->scaledBounds)/NSWidth(info->paperBounds));
      info->ypages = ceil(NSHeight(info->scaledBounds)/NSHeight(info->paperBounds));
      if ([_print_info horizontalPagination] == NSClipPagination)
        info->xpages = 1;
      if ([_print_info verticalPagination] == NSClipPagination)
        info->ypages = 1;
    }

  /*
   * Calculate nup. If nup is an odd multiple of two, secretly change the
   * page orientation to it's complement to make pages fit better.
   */
  if (((int)(info->nup / 2) & 0x1) == 1)
    {
      CGFloat tmp;
      if (info->orient == NSLandscapeOrientation)
        info->nupScale = 
          info->paperSize.width/(2*info->paperSize.height);
      else
        info->nupScale = 
          info->paperSize.height/(2*info->paperSize.width);
      info->nupScale /= (info->nup / 2);
      info->orient = (info->orient == NSPortraitOrientation) ? 
        NSLandscapeOrientation : NSPortraitOrientation;
      tmp = info->paperSize.width;
      info->paperSize.width = info->paperSize.height;
      info->paperSize.height = tmp;
      [dict setObject: NSNUMBER(info->orient) forKey: NSPrintOrientation];
    }
  else if (info->nup > 1)
    {
      info->nupScale = 2.0 / (CGFloat)info->nup;
    }

  if ([[dict objectForKey: NSPrintPageDirection] isEqual: @"Columns"])
    info->pageDirection = 1;
  else
    info->pageDirection = 0;
}

/* Our personnel method to calculate the print rect for the specified page.
   Note, we assume this function is called in order from our first to last
   page. The returned pageRect is in the view's coordinate system
*/
- (NSRect) _rectForPage: (int)page info: (page_info_t *)info 
                  xpage: (int *)xptr
                  ypage: (int *)yptr
{
  int xpage, ypage;
  NSRect pageRect;

  if (info->pageDirection == 1)
    {
      xpage = (page - 1) / info->ypages;
      ypage = (page - 1) % info->ypages;
    }
  else
    {
      xpage = (page - 1) % info->xpages;
      ypage = (page - 1) / info->xpages;
    }
  *xptr = xpage;
  *yptr = ypage;
  if (xpage == 0)
    info->lastWidth = 0;
  if (ypage == 0)
    info->lastHeight = 0;
  pageRect = NSMakeRect(info->lastWidth, info->lastHeight,
                        NSWidth(info->paperBounds), NSHeight(info->paperBounds));
  pageRect = NSIntersectionRect(pageRect, info->scaledBounds);
  /* Scale to view's coordinate system */
  return scaleRect(pageRect, 1/(info->pageScale*info->printScale));
  
}

/* Let the view adjust the page rect we calculated. See assumptions for
   _rectForPage:
*/
- (NSRect) _adjustPagesFirst: (int)first 
                        last: (int)last 
                        info: (page_info_t *)info
{
  int i, xpage, ypage;
  double hlimit, wlimit;
  NSRect pageRect = NSZeroRect; /* Silence compiler warning.  */
  hlimit = [_view heightAdjustLimit];
  wlimit = [_view widthAdjustLimit];
  for (i = first; i <= last; i++)
    {
      CGFloat newVal, limitVal;
      pageRect = [self _rectForPage: i info: info xpage: &xpage ypage: &ypage];
      limitVal = NSMaxY(pageRect) - hlimit * NSHeight(pageRect);
      [_view adjustPageHeightNew: &newVal
                             top: NSMinY(pageRect)
                          bottom: NSMaxY(pageRect)
                           limit: limitVal];
      if (newVal < NSMaxY(pageRect))
        pageRect.size.height = MAX(newVal, limitVal) - NSMinY(pageRect);
      limitVal = NSMaxX(pageRect) - wlimit * NSWidth(pageRect);
      [_view adjustPageWidthNew: &newVal
                           left: NSMinX(pageRect)
                          right: NSMaxX(pageRect)
                           limit: limitVal];
      if (newVal < NSMaxX(pageRect))
        pageRect.size.width = MAX(newVal, limitVal) - NSMinX(pageRect);
      if (info->pageDirection == 0 || ypage == info->ypages - 1)
        info->lastWidth = NSMaxX(pageRect)*(info->pageScale*info->printScale);
      if (info->pageDirection == 1 || xpage == info->xpages - 1)
        info->lastHeight = NSMaxY(pageRect)*(info->pageScale*info->printScale);
    }
  return pageRect;
}

- (void) _print
{
  int i, dir;
  BOOL knowsPageRange, allPages;
  NSRange viewPageRange = NSMakeRange(1, 0);
  NSMutableDictionary *dict;
  NSNumber *value;
  page_info_t info;
  
  dict = [_print_info dictionary];

  /* Setup pagination */
  knowsPageRange = [_view knowsPageRange: &viewPageRange]; 
  [self _printPaginateWithInfo: &info knowsRange: knowsPageRange];
  if (knowsPageRange == NO)
    {
      viewPageRange = NSMakeRange(1, (info.xpages * info.ypages));
    }
  else
    {
      // These values never get used
      info.xpages = 1;
      info.ypages = viewPageRange.length;
    }

  [dict setObject: NSNUMBER(NSMaxRange(viewPageRange))
           forKey: @"NSPrintTotalPages"];
  value = [dict objectForKey: NSPrintAllPages];
  if (value != nil)
    allPages = [value boolValue];
  else
    allPages = YES;
  if (allPages == YES)
    {
      info.first = viewPageRange.location;
      info.last = NSMaxRange(viewPageRange) - 1;
    }
  else
    {
      value = [dict objectForKey: NSPrintFirstPage];
      if (value != nil)
	info.first = [value intValue];
      else 
	info.first = 1;

      value = [dict objectForKey: NSPrintLastPage];
      if (value != nil)
	info.last  = [value intValue];
      else
	info.last  = INT_MAX;

      info.first = MAX(info.first, (int)viewPageRange.location);
      info.first = MIN(info.first, (int)(NSMaxRange(viewPageRange) - 1));
      info.last = MAX(info.last, info.first);
      info.last = MIN(info.last, (int)(NSMaxRange(viewPageRange) - 1));
      viewPageRange = NSMakeRange(info.first, (info.last-info.first)+1);
    }
  [dict setObject: NSFNUMBER(info.nupScale) forKey: @"NSNupScale"];
  [dict setObject: NSNUMBER(info.first) forKey: NSPrintFirstPage];
  if (allPages == YES && knowsPageRange == NO)
    [dict setObject: NSNUMBER(info.first-1) forKey: NSPrintLastPage];
  else
    [dict setObject: NSNUMBER(info.last) forKey: NSPrintLastPage];
  NSDebugLLog(@"NSPrinting", @"Printing pages %d to %d", 
              info.first, info.last);
  NSDebugLLog(@"NSPrinting", @"Printing rect %@, scaled %@",
              NSStringFromRect(_rect),
              NSStringFromRect(info.scaledBounds));

  if (_page_order == NSDescendingPageOrder)
    {
      _currentPage = info.last;
      dir = -1;
    }
  else
    {
      _currentPage = info.first;
      dir = 1;
    }

  /*
   * FIXME: Independent of the page order we could pre-calculate the
   * pageRects for all pages up to last (including clipping adjustment) 
   * and use them when printing. 
   */
  info.lastWidth = info.lastHeight = 0;
  if (!knowsPageRange && dir > 0 && _currentPage != 1)
    {
      /* Calculate page rects we aren't processing to catch up to the
         first page we are */
      [self _adjustPagesFirst: 1
                         last: _currentPage - 1 
                         info: &info];
    }

  /* Print the header information */
  [_view beginDocument];

  /* Print each page */
  i = 0;
  while (i < (info.last - info.first + 1))
    {
      NSRect pageRect;

      if (knowsPageRange == YES)
        {
          pageRect = [_view rectForPage: _currentPage];
        }
      else
        {
          if (dir < 0)
            pageRect = [self _adjustPagesFirst: 1 
                                          last: _currentPage 
                                          info: &info];
          else
            pageRect = [self _adjustPagesFirst: _currentPage 
                                          last: _currentPage 
                                          info: &info];
        }

      NSDebugLLog(@"NSPrinting", @" current page %d, rect %@", 
                  _currentPage, NSStringFromRect(pageRect));
      if (NSIsEmptyRect(pageRect))
        break;

      /* Draw using our special view routine */
      [_view _displayPageInRect: pageRect
	     withInfo: info
	     knowsPageRange: knowsPageRange];
             
      // We could end up in this case for each row/column not just the lase page.
      if (!knowsPageRange && dir > 0 && _currentPage == info.last && allPages == YES)
        {
          /* Check if adjust pages forced part of the bounds onto 
             another page */
          if (NSMaxX(pageRect) < NSMaxX(_rect) 
              && [_print_info horizontalPagination] != NSClipPagination)
            {
              info.xpages++;
            }
          if (NSMaxY(pageRect) < NSMaxY(_rect)
              && [_print_info verticalPagination] != NSClipPagination)
            {
              info.ypages++;
            }
          viewPageRange = NSMakeRange(1, (info.xpages * info.ypages));
          info.last = NSMaxRange(viewPageRange) - 1;
        }
      i++;
      _currentPage += dir;
    } /* Print each page */
  
  /* Make sure we end the sheet */
  if (info.nup > 1 && (info.last - info.first) % info.nup != info.nup - 1)
    {
      [_view drawSheetBorderWithSize: info.sheetBounds.size];
      [_view _endSheet];
      DPSgrestore([self context]);
    }
  [_view endDocument];

  /* Setup/reset for next time */
  [dict setObject: NSNUMBER(info.last) forKey: NSPrintLastPage];
  if (((int)(info.nup / 2) & 0x1) == 1)
    {
      info.orient = (info.orient == NSPortraitOrientation) ? 
        NSLandscapeOrientation : NSPortraitOrientation;
      [dict setObject: NSNUMBER(info.orient) forKey: NSPrintOrientation];
    }
}

@end

@implementation NSView (NSPrintOperation)

- (void) _displayPageInRect: (NSRect)pageRect
                   withInfo: (page_info_t)info
	     knowsPageRange: (BOOL)knowsPageRange
{
  int currentPage;
  int numberOnSheet;
  CGFloat xoffset, yoffset, scale;
  NSPoint location;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  currentPage = [printOp currentPage];
  numberOnSheet = (currentPage - info.first) % info.nup;

  /* Begin a sheet (i.e. a physical page in Postscript terms). If 
     nup > 1 then this occurs only once every nup pages */
  if (numberOnSheet == 0)
    {
      NSString *label;
      NSRect boundsForPage = info.sheetBounds;

      if (knowsPageRange)
	{
	  boundsForPage = pageRect;
	}

      label = nil;
      if (info.nup == 1)
        label = [NSString stringWithFormat: @"%d", currentPage];

      DPSgsave(ctxt);
      [self beginPage: floor((currentPage - info.first)/info.nup)+1
            label: label
            bBox: boundsForPage
            fonts: nil];
      if (info.orient == NSLandscapeOrientation)
        {
          DPSrotate(ctxt, 90);
          DPStranslate(ctxt, 0, -info.paperSize.height);
        }
      
      if (!knowsPageRange)
	{
	  /* Also offset by margins */
	  DPStranslate(ctxt, NSMinX(info.paperBounds), NSMinY(info.paperBounds));
	}

      /* End page setup for multi page */
      if (info.nup != 1)
        {
          [self addToPageSetup];
          [self endPageSetup];
        }
    }

  scale = info.pageScale * info.printScale;
  location = [self locationOfPrintRect: scaleRect(pageRect, scale)];

  /* Begin a logical page */
  [self beginPageInRect: pageRect atPlacement: location];
  if (scale != 1.0)
    DPSscale(ctxt, scale, scale);

  /* FIXME: Why is this needed? Shouldn't the flip be handled by the lockFocus method? */
  if ([self isFlipped])
    {
      NSAffineTransformStruct ats = { 1, 0, 0, -1, 0, NSHeight(_bounds) };
      NSAffineTransform *matrix, *flip;

      flip = [NSAffineTransform new];
      matrix = [NSAffineTransform new];
      if (_boundsMatrix != nil)
        {
          [matrix prependTransform: _boundsMatrix];
        }
      /*
       * The flipping process must result in a coordinate system that
       * exactly overlays the original. To do that, we must translate
       * the origin by the height of the view.
       */
      [flip setTransformStruct: ats];
      [matrix prependTransform: flip];
      [matrix concat];
      RELEASE(flip);
      RELEASE(matrix);
      yoffset = NSHeight(_frame) - NSMaxY(pageRect);
    }
  else
    yoffset = 0 - NSMinY(pageRect);

  /* Translate so the rect we're printing is on the page */
  xoffset = 0 - NSMinX(pageRect);
  DPStranslate(ctxt, xoffset, yoffset);

  /* End page setup for single page */
  if (info.nup == 1)
    {
      [self addToPageSetup];
      [self endPageSetup];
    }

  /* Do the actual drawing */
  [self displayRectIgnoringOpacity: pageRect inContext: ctxt];

  /* End a logical page */
  // FIXME: Attempt to get the coordinates of the page border correct.
  DPSgrestore(ctxt);
  DPSgsave(ctxt);
  [self drawPageBorderWithSize: info.paperBounds.size];
  [self endPage];

  /* End a physical page */
  if (numberOnSheet == info.nup - 1)
    {
      [self drawSheetBorderWithSize: info.sheetBounds.size];
      [self _endSheet];
      DPSgrestore(ctxt);
    }
}

@end
